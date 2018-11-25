#ifndef __FIND__
#define __FIND__

#include <arpa/inet.h>

struct route {
    struct in_addr ip_prefix;
    uint32_t prefix_len;
    struct nexthop *nexthop;
};

struct remote_host_t {
    char *if_name;
    uint32_t if_index;
    struct in_addr addr;
};

struct nexthop {
    struct nexthop *next;
    struct remote_host_t host;
};

struct nextaddr {
    struct remote_host_t host;
    uint32_t prefix_len;
};

void init_route();
int insert_route(struct in_addr ip_prefix, uint32_t prefix_len, char *if_name,
                 uint32_t if_index, struct in_addr nexthop_addr);
int lookup_route(struct in_addr dst_addr, struct nextaddr *nexthop_info);
int delete_route(struct in_addr dst_addr, uint32_t prefix_len);

#endif
