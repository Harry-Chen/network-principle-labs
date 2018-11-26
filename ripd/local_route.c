#include "local_route.h"
#include "routing_table.h"
#include "common.h"


static if_info_t if_info[MAX_IF];

void init_local_interfaces() {
    struct ifaddrs *ifaddr, *ifa;
    int family;

    bzero(if_info, sizeof(if_info));

    if (getifaddrs(&ifaddr) == -1) {
        fprintf(stderr, "Failed to get info of local interface\n");
        exit(EXIT_FAILURE);
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL)
            continue;
 
        family = ifa->ifa_addr->sa_family;
  
        // filter only interfaces with IPv4 address and is not lo & support multicast
        if (family == AF_INET && !(ifa->ifa_flags & IFF_LOOPBACK)) {
            uint32_t if_index = if_nametoindex(ifa->ifa_name);

            char ip_addr[INET_ADDRSTRLEN];
            printf("[Local Route] Interface %d: %s", if_index, ifa->ifa_name);
            struct in_addr *addr = &((struct sockaddr_in *) ifa->ifa_addr)->sin_addr;
            struct in_addr *mask = &((struct sockaddr_in *) ifa->ifa_netmask)->sin_addr;
            inet_ntop(AF_INET, &(addr->s_addr), ip_addr, INET_ADDRSTRLEN);
            uint32_t prefix_len = PREFIX_BIN2DEC(ntohl(mask->s_addr));
            printf(" with address: %s/%d\n", ip_addr, prefix_len);

            // insert link-scope routes
            TRtEntry local_route_entry = {
                .stIpPrefix = *addr,
                .uiPrefixLen = prefix_len,
                .stNexthop = *addr,
                .uiMetric = 1,
                .uiInterfaceIndex = if_index
            };
            insert_route_local(&local_route_entry);


            strcpy(if_info[if_index].name, ifa->ifa_name);
            if_info[if_index].ip = *addr;
            if_info[if_index].prefix_len = prefix_len;
            if_info[if_index].multicast = (ifa->ifa_flags & IFF_MULTICAST) != 0;
        }
    }
    print_all_routes(stderr);
    freeifaddrs(ifaddr);
}

if_info_t *get_interface_info(uint8_t index) {
    return &if_info[index];
}