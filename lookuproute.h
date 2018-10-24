#ifndef __FIND__
#define __FIND__
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct route {
    struct in_addr ip_prefix;
    unsigned int prefix_len;
    struct nexthop *nexthop;
};

struct nexthop {
    struct nexthop *next;
    char *if_name;
    unsigned int if_index;
    struct in_addr addr;
};

struct nextaddr {
    char *if_name;
    struct in_addr addr;
    unsigned int prefix_len;
};

void init_route();
int insert_route(unsigned long ip_prefix, unsigned int prefix_len, char *if_name,
                 unsigned int if_index, unsigned long nexthop_addr);
int lookup_route(struct in_addr dst_addr, struct nextaddr *nexthop_info);
int delete_route(struct in_addr dst_addr, unsigned int prefix_len);

#endif
