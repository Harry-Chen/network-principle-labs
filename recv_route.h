#ifndef __RECVROUTE__
#define __RECVROUTE__

#include <stdint.h>

#include <arpa/inet.h>
#include <net/if.h>

struct selfroute {
    uint32_t prefixlen;
    struct in_addr prefix;
    uint32_t ifindex;
    struct in_addr nexthop;
    uint32_t cmdnum;
    char ifname[IFNAMSIZ];
} buf2;

int static_route_get(struct selfroute *selfrt);
#endif
