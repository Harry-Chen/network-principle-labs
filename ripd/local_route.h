#ifndef __LOCAL_ROUTE_H__
#define __LOCAL_ROUTE_H__


#include "common.h"

#define MAX_IF 256

typedef struct {
    char name[IF_NAMESIZE];
    uint8_t multicast;
    struct in_addr ip;
    uint32_t prefix_len;
} if_info_t;

void init_local_interfaces();

if_info_t *get_interface_info(uint8_t index);

extern struct in_addr NEXTHOP_ONLINK;

#endif