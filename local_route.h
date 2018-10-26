#ifndef __LOCAL_ROUTE_H__
#define __LOCAL_ROUTE_H__

#include "query_mac.h"
#include <net/if.h>
#include <netinet/in.h>

#define MAX_IF 256

struct if_info_t {
    char name[IF_NAMESIZE];
    in_addr_t ip;
    uint32_t prefix_len;
    macaddr_t mac;
};

void init_local_interfaces();

#endif