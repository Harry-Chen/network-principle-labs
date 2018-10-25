
#include "arp_find.h"
#include "check_sum.h"
#include "lookup_route.h"
#include "recv_route.h"
#include "send_ether_ip.h"
#include <pthread.h>

#define IP_HEADER_LEN sizeof(struct ip)
#define ETHER_HEADER_LEN sizeof(struct ether_header)

// thread to receive routing table change
void *thr_fn(void *arg) {
    int st = 0;
    struct selfroute *selfrt;
    selfrt = (struct selfroute *)malloc(sizeof(struct selfroute));
    memset(selfrt, 0, sizeof(struct selfroute));

    // get if.name
    struct if_nameindex *head, *ifni;
    ifni = if_nameindex();
    head = ifni;
    char *ifname;

    // add-24 del-25
    while (1) {
        st = static_route_get(selfrt);
        if (st == 1) {
            if (selfrt->cmdnum == 24) {
                while (ifni->if_index != 0) {
                    if (ifni->if_index == selfrt->ifindex) {
                        printf("if_name is %s\n", ifni->if_name);
                        ifname = ifni->if_name;
                        break;
                    }
                    ifni++;
                }

                // TODO: insert to routing table

            } else if (selfrt->cmdnum == 25) {
                // TODO: delete from routing table
            }
        }
    }
}

int main() {

    char skbuf[1500];
    int recvfd, recvlen, datalen;
    struct ip *ip_recv_header;
    pthread_t tid;
    ip_recv_header = (struct ip *)malloc(sizeof(struct ip));
    char ip_addr_from[INET_ADDRSTRLEN], ip_addr_to[INET_ADDRSTRLEN];

    // use raw socket to capture ip packets
    if ((recvfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_IP))) == -1) {
        printf("Error opening raw socket for IP packet\n");
        return -1;
    }

    // initialize routing table
    init_route();

    // TODO: insert link routes to routing table

    // use thread to receive routing table change from quagga
    int pd = pthread_create(&tid, NULL, thr_fn, NULL);

    struct nextaddr *nexthopinfo = (struct nextaddr *) malloc(sizeof(struct nextaddr));
    macaddr_t mac_addr;
    
    while (1) {
        recvlen = recv(recvfd, skbuf, sizeof(skbuf), 0);
        if (recvlen > 0) {

            ip_recv_header = (struct ip *)(skbuf + ETHER_HEADER_LEN);
            datalen = recvlen - ETHER_IP_LEN;
            inet_ntop(AF_INET, &(ip_recv_header->ip_src.s_addr), ip_addr_from, INET_ADDRSTRLEN);
            inet_ntop(AF_INET, &(ip_recv_header->ip_dst.s_addr), ip_addr_to, INET_ADDRSTRLEN);

            uint16_t length_in_header = ntohs(ip_recv_header->ip_len);

            printf("Received IP packet from %s to %s, with payload length %d.\n", ip_addr_from, ip_addr_to, datalen);
            
            if (length_in_header != datalen) {
                printf("Payload length in ip header does not match data received, should be %d\n", length_in_header);
                continue;
            }

            // 192.168.1.10是测试服务器的IP，现在测试服务器IP是192.168.1.10到192.168.1.80.
            //使用不同的测试服务器要进行修改对应的IP。然后再编译。
            // 192.168.6.2是测试时候ping的目的地址。与静态路由相对应。
            // if (ip_recv_header->ip_src.s_addr == inet_addr("192.168.1.10") &&
            //     ip_recv_header->ip_dst.s_addr == inet_addr("192.168.6.2")) {

                uint16_t result = calculate_check_sum(ip_recv_header);
                printf("Checksum is %x", result);

                if (result != ip_recv_header->ip_sum) {
                    printf(", should be %x.\n", ip_recv_header->ip_sum);
                    continue;
                }
                printf(", OK!\n");

                result = lookup_route(ip_recv_header->ip_dst, nexthopinfo);

                if (result == 1) {
                    printf("Route not found for \n");
                    continue;
                }


                // TODO: get MAC address from ARP table
                // TODO: construct ip header
                // TODO: construct ethernet header
                // TODO: fill in data
                // TODO: send by raw socket

            // }
        }
    }

    close(recvfd);
    return 0;
}
