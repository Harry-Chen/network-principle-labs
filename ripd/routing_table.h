#ifndef __ROUTING_TABLE__
#define __ROUTING_TABLE__

#include <stdio.h>
#include <arpa/inet.h>
#include "common.h"

#define CMD_ADD 24
#define CMD_DEL 25

void init_route();
void notify_forwarder(TRtEntry *entry, uint32_t cmd);
void insert_route_local(TRtEntry *entry);
void insert_route_rip(TRipEntry *entry);
TRtEntry *lookup_route_longest(struct in_addr dst_addr);
TRtEntry *lookup_route_exact(struct in_addr dst_addr, uint32_t prefix);
void delete_route_rip(TRtEntry *entry);
int fill_rip_packet(TRipEntry *rip_entry, struct in_addr iface_addr);
void print_all_routes(FILE* f);

#endif
