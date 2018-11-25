#ifndef __ROUTING_TABLE__
#define __ROUTING_TABLE__

#include <arpa/inet.h>
#include "common.h"

void init_route();
void insert_route(TRtEntry *entry);
TRtEntry *lookup_route_exact(struct in_addr dst_addr, uint32_t prefix);
void delete_route(TRtEntry *entry);
int fill_rip_packet(TRipEntry *rip_entry, struct in_addr iface_addr);

#endif
