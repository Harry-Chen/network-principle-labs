#ifndef __LOCAL_ROUTE_H__
#define __LOCAL_ROUTE_H__

#include "common.h"

#define MAX_IF 256

typedef struct {
    char name[IF_NAMESIZE];
    bool if_valid;
    bool if_up;
    bool multicast;
    struct in_addr ip;
    uint32_t prefix_len;
} if_info_t;

void init_local_interfaces();

if_info_t *get_interface_info(uint8_t index);

bool is_local_address(struct in_addr addr);

void update_interface_info();

#endif