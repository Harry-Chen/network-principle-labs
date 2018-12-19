#include "local_route.h"
#include "routing_table.h"
#include "common.h"


static if_info_t if_info[MAX_IF];

static void update_local_interfaces(int i) {
    struct ifaddrs *ifaddr, *ifa;
    int family;

    if_info_t *info = &if_info[i];

    if (getifaddrs(&ifaddr) == -1) {
        fprintf(stderr, "Failed to get info of local interface\n");
        exit(EXIT_FAILURE);
    }

    bool found = false;

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL)
            continue;

        if (strcmp(info->name, ifa->ifa_name) != 0)
            continue;
 
        family = ifa->ifa_addr->sa_family;
  
        if (family == AF_INET) { // found up interfaces

            found = true;

            // interface state not changed
            if (info->if_up) { 
                continue;
            }

            if (info->ip.s_addr != 0) {
                // interface initialized, but state changed from DOWN to UP
                TRtEntry *entry = lookup_route_longest(info->ip);
                entry->uiMetric = 1;
                printf("[Local Route] Interface %s changed to UP, notifying forwarder...\n", info->name);
                notify_forwarder(entry, CMD_ADD);
                continue;
            }

            // interface with new ip discovered
            char ip_addr[INET_ADDRSTRLEN];
            struct in_addr *addr = &((struct sockaddr_in *) ifa->ifa_addr)->sin_addr;
            struct in_addr *mask = &((struct sockaddr_in *) ifa->ifa_netmask)->sin_addr;
            inet_ntop(AF_INET, &(addr->s_addr), ip_addr, INET_ADDRSTRLEN);
            uint32_t prefix_len = PREFIX_BIN2DEC(ntohl(mask->s_addr));
            printf("[Local Route] Interface %s is UP with address: %s/%d\n", ifa->ifa_name, ip_addr, prefix_len);

            // insert link-scope routes
            TRtEntry local_route_entry = {
                .stIpPrefix = *addr,
                .uiPrefixLen = prefix_len,
                .stNexthop = *addr,
                .uiMetric = 1,
                .uiInterfaceIndex = i,
                .isRip = false
            };
            insert_route_local(&local_route_entry);


            info->if_up = true;
            info->ip = *addr;
            info->prefix_len = prefix_len;
            info->multicast = (ifa->ifa_flags & IFF_MULTICAST) != 0;

            break;
        }
    }

    // link state DOWN
    if (!found) {
        // interface not initialized
        if (info->ip.s_addr == 0) return;
        // status not changed
        if (!info->if_up) return;
        // status changed from UP to DOWN
        TRtEntry *entry = lookup_route_longest(info->ip);
        entry->uiMetric = 16;
        printf("[Local Route] Interface %s changed to DOWN, notifying forwarder...\n", info->name);
        notify_forwarder(entry, CMD_DEL);
        info->if_up = false;
    }

    freeifaddrs(ifaddr);
}

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
        uint32_t if_index = if_nametoindex(ifa->ifa_name);

        if (family == AF_PACKET && !(ifa->ifa_flags & IFF_LOOPBACK)) {
            printf("[Local Route] Found Interface %s\n", ifa->ifa_name);
            strcpy(if_info[if_index].name, ifa->ifa_name);
            if_info[if_index].if_valid = true;
            if_info[if_index].if_up = false;
        }
    }
    freeifaddrs(ifaddr);

    update_interface_info();

    print_all_routes(stderr);
}

if_info_t *get_interface_info(uint8_t index) {
    return &if_info[index];
}

void update_interface_info() {
    for (int i = 0; i < MAX_IF; ++i) {
        if (if_info[i].if_valid) {
            update_local_interfaces(i);
        }
    }
}

bool is_local_address(struct in_addr addr) {
    for (int i = 0; i < MAX_IF; ++i) {
        if_info_t *iface = get_interface_info(i);
        if (iface->name[0] == '\0') continue; // empty interface
        if (iface->ip.s_addr == addr.s_addr) return true;
    }
    return 0;
}