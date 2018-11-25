#include "routing_table.h"
#include "../routing_table/rt.h"

static routing_table_t routing_table;

#define MAX_TABLE_SIZE (1<<20)
static TRtEntry *table[MAX_TABLE_SIZE];
static int table_size = 1;


void init_route() {
    routing_table = rt_init();
}

void insert_route(TRtEntry *entry) {
    TRtEntry *item = (TRtEntry*) malloc(sizeof(TRtEntry));
    memcpy(item, entry, sizeof(TRtEntry));
    table[table_size] = item;
    rt_insert(routing_table, ntohl(item->stIpPrefix.s_addr), item->uiPrefixLen, table_size);
    table_size++;
}

TRtEntry *lookup_route_exact(struct in_addr dst_addr, uint32_t prefix) {
    uint32_t rt_index = rt_match(routing_table, ntohl(dst_addr.s_addr), prefix, 1);

    if (rt_index == 0) {
        return NULL;
    } else {
        return table[rt_index];
    }
}

void delete_route(TRtEntry *entry) {
    rt_remove(routing_table, ntohl(entry->stIpPrefix.s_addr), entry->uiPrefixLen);
}


int fill_rip_packet(TRipEntry *rip_entry) {
    int size = 0;
    int index = 0;
    while ((index = rt_iterate(routing_table, index)) != -1) {
        TRtEntry *rt_entry = table[index];
        rip_entry[size].usFamily = htons(AF_INET);
        rip_entry[size].usTag = 0;
        rip_entry[size].stAddr = rt_entry->stIpPrefix;
        rip_entry[size].stPrefixLen.s_addr = htonl(((~0) >> (32 - rt_entry->uiPrefixLen)) << (32 - rt_entry->uiPrefixLen));
        rip_entry[size].stNexthop = rt_entry->stNexthop;
        rip_entry[size].uiMetric = rt_entry->uiMetric;
        ++size;
    }

    return size;
}