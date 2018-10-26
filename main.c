
#include "recv_route.h"
#include "check_sum.h"
#include "lookup_route.h"
#include "query_mac.h"
#include "send_ether_ip.h"
#include <pthread.h>

#define IP_HEADER_LEN sizeof(struct ip)
#define ETHER_HEADER_LEN sizeof(struct ether_header)

#define DEBUG(...) printf(__VA_ARGS__)

// thread to receive routing table change
void *receive_rt_change(void *arg) {
    DEBUG("Thread started to receive routing table change.\n");
    int st = 0;
    struct selfroute selfrt;
    char ifname[IF_NAMESIZE];

    // add-24 del-25
    while (1) {
        st = static_route_get(&selfrt);
        if (st == 1) {
            if_indextoname(selfrt.ifindex, ifname);
            if (selfrt.cmdnum == 24) {
                // TODO: insert to routing table
            } else if (selfrt.cmdnum == 25) {
                // TODO: delete from routing table
            }
        }
    }
}

int main() {

    char skbuf[1500];
    int recvfd, recvlen, datalen;

    // use raw socket to capture ip packets
    if ((recvfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_IP))) == -1) {
        printf("Error opening raw socket for IP packet\n");
        return -1;
    }

    // initialize routing table
    init_route();

    // TODO: insert link routes to routing table

    // use thread to receive routing table change from quagga
    pthread_t tid;
    int pd = pthread_create(&tid, NULL, receive_rt_change, NULL);
    
    while (1) {
        recvlen = recv(recvfd, skbuf, sizeof(skbuf), 0);
        if (recvlen > 0) {
            
            // cast to header type
            struct ethhdr *eth_header = (struct ethhdr*) skbuf;
            struct ip *ip_recv_header = (struct ip *)(skbuf + ETHER_HEADER_LEN);


            // analyze IP packet
            char ip_addr_from[INET_ADDRSTRLEN], ip_addr_to[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(ip_recv_header->ip_src.s_addr), ip_addr_from, INET_ADDRSTRLEN);
            inet_ntop(AF_INET, &(ip_recv_header->ip_dst.s_addr), ip_addr_to, INET_ADDRSTRLEN);
            uint16_t header_length = ip_recv_header->ip_hl * 4;
            datalen = recvlen - ETHER_HEADER_LEN - header_length;
            DEBUG("\nReceived IP packet from %s to %s, with payload length %d.\n", ip_addr_from, ip_addr_to, datalen);


            // verify checksum
            uint16_t result = calculate_check_sum(ip_recv_header);
            DEBUG("Checksum is %x", result);

            if (result != ip_recv_header->ip_sum) {
                DEBUG(", should be %x.\n", ip_recv_header->ip_sum);
                continue;
            }
            DEBUG(", OK!\n");


            // lookup next hop in routing table
            struct nextaddr nexthopinfo;
            result = lookup_route(ip_recv_header->ip_dst, &nexthopinfo);

            if (result == 1) {
                DEBUG("Route not found for %s\n", ip_addr_to);
                continue;
            }

            if (nexthopinfo.addr.s_addr == 0u) {
                // nexthop "0.0.0.0" means onlink
                nexthopinfo.addr.s_addr = ip_recv_header->ip_dst.s_addr;
            }

            if (nexthopinfo.addr.s_addr == UINT32_MAX) {
                // nexthop "255.255.255.255" means ignoring
                DEBUG("Packet to local address, ignored.\n");
                continue;
            }

            inet_ntop(AF_INET, &(nexthopinfo.addr), ip_addr_from, INET_ADDRSTRLEN);
            DEBUG("Next hop is %s via %s, with prefix length %d\n", ip_addr_from, nexthopinfo.if_name, nexthopinfo.prefix_len);


            // construct ip header
            if (--ip_recv_header->ip_ttl == 0) {
                DEBUG("TTL decreased to 0, goodbye.\n");
                continue;
            }
            uint16_t new_checksum = calculate_check_sum(ip_recv_header);
            DEBUG("New checksum of packet is %x\n", new_checksum);
            ip_recv_header->ip_sum = new_checksum;


            // get MAC address of next hop from ARP table
            macaddr_t mac_addr_to, mac_addr_from;
            result = arp_get_mac(mac_addr_to, nexthopinfo.if_name, ip_addr_from);

            if (result == 1) {
                DEBUG("MAC Address for next hop not in the ARP cache.\n");
                continue;
            }

            DEBUG("Destination MAC Address: %02x:%02x:%02x:%02x:%02x:%02x\n",
                mac_addr_to[0], mac_addr_to[1], mac_addr_to[2], mac_addr_to[3], mac_addr_to[4], mac_addr_to[5]);


            //get MAC address of source interface
            result = if_get_mac(mac_addr_from, nexthopinfo.if_name);

            DEBUG("Source MAC Address: %02x:%02x:%02x:%02x:%02x:%02x\n",
                mac_addr_from[0], mac_addr_from[1], mac_addr_from[2], mac_addr_from[3], mac_addr_from[4], mac_addr_from[5]);
            

            // fill in ethernet header
            memcpy(eth_header->h_dest, mac_addr_to, ETH_ALEN);
            memcpy(eth_header->h_source, mac_addr_from, ETH_ALEN);


            // we do not touch the payload of ip packet


            // TODO: send by raw socket
        }
    }

    pthread_cancel(tid);
    close(recvfd);
    return 0;
}
