
#include "arp_find.h"
#include "check_sum.h"
#include "lookup_route.h"
#include "recv_route.h"
#include "send_ether_ip.h"
#include <pthread.h>

#define IP_HEADER_LEN sizeof(struct ip)
#define ETHER_HEADER_LEN sizeof(struct ether_header)

#define DEBUG(...) printf(__VA_ARGS__)

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

    struct nextaddr nexthopinfo;
    macaddr_t mac_addr;
    
    while (1) {
        recvlen = recv(recvfd, skbuf, sizeof(skbuf), 0);
        if (recvlen > 0) {

            ip_recv_header = (struct ip *)(skbuf + ETHER_HEADER_LEN);
            inet_ntop(AF_INET, &(ip_recv_header->ip_src.s_addr), ip_addr_from, INET_ADDRSTRLEN);
            inet_ntop(AF_INET, &(ip_recv_header->ip_dst.s_addr), ip_addr_to, INET_ADDRSTRLEN);

            uint16_t header_length = ip_recv_header->ip_hl * 4;

            datalen = recvlen - ETHER_HEADER_LEN - header_length;

            DEBUG("Received IP packet from %s to %s, with payload length %d.\n", ip_addr_from, ip_addr_to, datalen);

            // 192.168.1.10是测试服务器的IP，现在测试服务器IP是192.168.1.10到192.168.1.80.
            //使用不同的测试服务器要进行修改对应的IP。然后再编译。
            // 192.168.6.2是测试时候ping的目的地址。与静态路由相对应。
            // if (ip_recv_header->ip_src.s_addr == inet_addr("192.168.1.10") &&
            //     ip_recv_header->ip_dst.s_addr == inet_addr("192.168.6.2")) {

                uint16_t result = calculate_check_sum(ip_recv_header);
                DEBUG("Checksum is %x", result);

                if (result != ip_recv_header->ip_sum) {
                    DEBUG(", should be %x.\n", ip_recv_header->ip_sum);
                    continue;
                }
                DEBUG(", OK!\n");

                result = lookup_route(ip_recv_header->ip_dst, &nexthopinfo);

                if (result == 1) {
                    DEBUG("Route not found for %s\n", ip_addr_to);
                    continue;
                }

                inet_ntop(AF_INET, &(nexthopinfo.addr), ip_addr_from, INET_ADDRSTRLEN);
                DEBUG("Next hop is %s via %s, with prefix length %d\n", ip_addr_from, nexthopinfo.if_name, nexthopinfo.prefix_len);


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
