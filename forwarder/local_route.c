#include "local_route.h"
#include "routing_table.h"

#include "common.h"

extern bool local_interface;

static struct if_info_t if_info[MAX_IF];

void init_local_interfaces() {
    struct ifaddrs *ifaddr, *ifa;
    int family;

    if (getifaddrs(&ifaddr) == -1) {
        fprintf(stderr, "Failed to get info of local interface\n");
        exit(EXIT_FAILURE);
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL)
            continue;
 
        family = ifa->ifa_addr->sa_family;
  
        // need only interfaces with IPv4 address and is not lo
        if (family == AF_INET && !(ifa->ifa_flags & IFF_LOOPBACK)) {
            uint32_t if_index = if_nametoindex(ifa->ifa_name);

            char ip_addr[INET_ADDRSTRLEN];
            printf("Found interface %d: %s", if_index, ifa->ifa_name);
            struct in_addr *addr = &((struct sockaddr_in *) ifa->ifa_addr)->sin_addr;
            struct in_addr *mask = &((struct sockaddr_in *) ifa->ifa_netmask)->sin_addr;
            uint32_t prefix = ntohl(mask->s_addr);
            inet_ntop(AF_INET, &(addr->s_addr), ip_addr, INET_ADDRSTRLEN);
            uint32_t prefix_len = 32 - __builtin_ctz(prefix);
            printf(" with IPv4 address: %s/%d\n", ip_addr, prefix_len);

            if (local_interface) {
                // insert link-scope and local-address routes
                insert_route(*addr, 32, ifa->ifa_name, if_index, NEXTHOP_SELF);
                insert_route(*addr, prefix_len, ifa->ifa_name, if_index, NEXTHOP_ONLINK);
            }

            strcpy(if_info[if_index].name, ifa->ifa_name);
            if_info[if_index].ip = addr->s_addr;
        }

        else if (family == AF_PACKET && !(ifa->ifa_flags & IFF_LOOPBACK)) {
            uint32_t if_index = if_nametoindex(ifa->ifa_name);
            struct sockaddr_ll *s = (struct sockaddr_ll*)(ifa->ifa_addr);
            printf("Found interface %d: %s", if_index, ifa->ifa_name);
            printf(" with MAC address: %02x:%02x:%02x:%02x:%02x:%02x\n", s->sll_addr[0], s->sll_addr[1], s->sll_addr[2], s->sll_addr[3], s->sll_addr[4], s->sll_addr[5]);
            memcpy(if_info[if_index].mac, s->sll_addr, ETH_ALEN);
        }
    }
 
    freeifaddrs(ifaddr);
}

void get_mac_interface(macaddr_t **mac, uint32_t if_index) {
    *mac = &(if_info[if_index].mac);
}