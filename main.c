
#include "recv_route.h"
#include "check_sum.h"
#include "routing_table.h"
#include "arp_query.h"
#include "local_route.h"

#include "common.h"

#include <time.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>

#ifdef VERBOSE
#define DEBUG(...) printf(__VA_ARGS__)
#else
#define DEBUG(...) do{} while(0)
#endif

#define BUF_SIZE 65535

int should_exit = 0;

void signal_handler(int signo) {
    if (signo == SIGINT) {
        printf("Received SIGINT, exiting...\n");
        should_exit = 1;
    }
}

int main() {

    // 1500 BYTES IS NOT ENOUGH! DON'T TRUST TA!
    char skbuf[BUF_SIZE];
    int recvfd, sendfd, arp_fd;
    uint16_t recvlen, datalen;
    unsigned long long forward_length_total = 0;
    unsigned long long recv_count = 0, forward_count = 0;
    time_t last_print = time(NULL);
    time_t now;

    // use raw socket to capture and send ip packets
    if ((recvfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_IP))) == -1) {
        fprintf(stderr, "Error opening raw socket for capturing IP packet: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if ((sendfd = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW)) == -1) {
        fprintf(stderr, "Error opening raw socket for sending IP packet: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if ((arp_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        fprintf(stderr, "Error opening socket for querying ARP table: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }


    // initialize routing table
    init_route();

    // insert link routes to routing table
    init_local_interfaces();

    // use thread to receive routing table change from quagga
    pthread_t tid;
    int pd;
    if ((pd = pthread_create(&tid, NULL, receive_rt_change, NULL)) < 0) {
        fprintf(stderr, "Error creating thread.\n");
        exit(EXIT_FAILURE);
    } else {
        printf("Thread started to receive routing table change.\n");
    }

    if (signal(SIGINT, signal_handler) == SIG_ERR) {
        fprintf(stderr, "Error registering signal handler!\n");
        exit(EXIT_FAILURE);
    } else {
        printf("Starting forwarding...\n");
    }
    
    while (!should_exit) {

        // print statistics
        if ((now = time(NULL)) - last_print == 1) {
            printf("\33[2K\rReceived %llu packets, forwarded %llu packets. Speed: %llu Mbps/s\n", recv_count, forward_count, forward_length_total / 1024 / 1024 * 8);
            last_print = now;
            forward_length_total = 0;
        }

        recvlen = recv(recvfd, skbuf, sizeof(skbuf), 0);
        if (recvlen > 0) {
            recv_count += 1;
            // cast to header type
            struct ethhdr *eth_header = (struct ethhdr*) skbuf;
            struct ip *ip_recv_header = (struct ip *)(skbuf + sizeof(struct ether_header));


            // analyze IP packet
            char ip_addr_from[INET_ADDRSTRLEN], ip_addr_to[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(ip_recv_header->ip_src.s_addr), ip_addr_from, INET_ADDRSTRLEN);
            inet_ntop(AF_INET, &(ip_recv_header->ip_dst.s_addr), ip_addr_to, INET_ADDRSTRLEN);
            uint16_t header_length = ip_recv_header->ip_hl * 4;
            datalen = recvlen - sizeof(struct ether_header) - header_length;
            DEBUG("\nReceived IP packet from %s to %s, with payload length %d.\n", ip_addr_from, ip_addr_to, datalen);
            if (datalen > 1500) {
                DEBUG("Packet too large (>MTU), ignored.\n");
                continue;
            }
            uint16_t result;

#ifndef SPEEDUP
            // verify checksum
            result = calculate_check_sum(ip_recv_header);
            DEBUG("Checksum is %x", result);

            if (result != ip_recv_header->ip_sum) {
                DEBUG(", should be %x.\n", ip_recv_header->ip_sum);
                continue;
            }
            DEBUG(", OK!\n");
#endif


            // lookup next hop in routing table
            struct nextaddr nexthopinfo;
            result = lookup_route(ip_recv_header->ip_dst, &nexthopinfo);

            if (result == 1) {
                DEBUG("Route not found for %s\n", ip_addr_to);
                continue;
            }

            if (nexthopinfo.host.addr.s_addr == NEXTHOP_ONLINK.s_addr) {
                nexthopinfo.host.addr.s_addr = ip_recv_header->ip_dst.s_addr;
            }

            if (nexthopinfo.host.addr.s_addr == NEXTHOP_SELF.s_addr) {
                DEBUG("Packet to local address, ignored.\n");
                continue;
            }

            inet_ntop(AF_INET, &(nexthopinfo.host.addr), ip_addr_from, INET_ADDRSTRLEN);
            DEBUG("Next hop is %s via %s, with prefix length %d\n", ip_addr_from, nexthopinfo.host.if_name, nexthopinfo.prefix_len);

            // construct ip header
            if (--ip_recv_header->ip_ttl == 0) {
                DEBUG("TTL decreased to 0, goodbye.\n");
                continue;
            }
            
#ifndef SPEEDUP
            // calculate new checksum
            uint16_t new_checksum = calculate_check_sum(ip_recv_header);
            DEBUG("New checksum of packet is %x\n", new_checksum);
            ip_recv_header->ip_sum = new_checksum;
#else
            ++ip_recv_header->ip_sum;
#endif


            // get MAC address of next hop from ARP table
            macaddr_t mac_addr_to, *mac_addr_from;
            result = arp_get_mac(arp_fd, mac_addr_to, nexthopinfo.host.if_name, ip_addr_from);

            if (result == 2) {
                DEBUG("Lookup ARP table failed, maybe next hop is unreachable or myself?\n");
                continue;
            } else if (result == 1) {
                DEBUG("MAC Address for next hop not in the ARP cache.\n");
                continue;
            }

            DEBUG("Destination MAC Address: %02x:%02x:%02x:%02x:%02x:%02x\n",
                mac_addr_to[0], mac_addr_to[1], mac_addr_to[2], mac_addr_to[3], mac_addr_to[4], mac_addr_to[5]);


            //get MAC address of source interface
            get_mac_interface(&mac_addr_from, nexthopinfo.host.if_index);

            DEBUG("Source MAC Address: %02x:%02x:%02x:%02x:%02x:%02x\n",
                *mac_addr_from[0], *mac_addr_from[1], *mac_addr_from[2], *mac_addr_from[3], *mac_addr_from[4], *mac_addr_from[5]);
            

            // fill in ethernet header
            memcpy(eth_header->h_dest, mac_addr_to, ETH_ALEN);
            memcpy(eth_header->h_source, *mac_addr_from, ETH_ALEN);


            // we do not touch the payload of ip packet


            // send by raw socket
            struct sockaddr_ll sadr_ll;
            sadr_ll.sll_ifindex = nexthopinfo.host.if_index;
            sadr_ll.sll_halen = ETH_ALEN;
            memcpy(sadr_ll.sll_addr, mac_addr_to, ETH_ALEN);

            if ((result = sendto(sendfd, skbuf, recvlen, 0, (const struct sockaddr *) &sadr_ll, sizeof(struct sockaddr_ll))) == -1) {
                fprintf(stderr, "Error sending raw IP packet: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            } else {
                DEBUG("Send succeeded!\n");
                forward_length_total += recvlen;
                forward_count += 1;
            }

        }
    }

    pthread_cancel(tid);
    close(recvfd);
    close(sendfd);
    close(arp_fd);

    return EXIT_SUCCESS;
}
