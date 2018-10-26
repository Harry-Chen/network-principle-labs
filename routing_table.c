#include "routing_table.h"
#include "routing_table/rt.h"

#include <string.h>
#include <stdlib.h>

static routing_table_t routing_table;

#define MAX_TABLE_SIZE (1<<20)
static struct route *table[MAX_TABLE_SIZE];
static int table_size = 1;

void init_route() {
    routing_table = rt_init();
}

int insert_route(uint32_t ip_prefix, uint32_t prefix_len, char *if_name,
                 uint32_t if_index, uint32_t nexthop_addr) {

    struct route* item = (struct route*) malloc(sizeof(struct route));
    (item->ip_prefix).s_addr = ip_prefix;
    item->prefix_len = prefix_len;
    item->nexthop = (struct nexthop*) malloc(sizeof(struct nexthop));
    item->nexthop->host.if_name = (char *) malloc(strlen(if_name));
    strcpy(item->nexthop->host.if_name, if_name);
    item->nexthop->host.if_index = if_index;
    item->nexthop->host.addr.s_addr = nexthop_addr;

    table[table_size] = item;
    rt_insert(routing_table, ntohl(ip_prefix), prefix_len, table_size);
    table_size++;

    return 0;
}

int lookup_route(struct in_addr dst_addr, struct nextaddr *nexthop_info) {

    uint32_t rt_index = rt_lookup(routing_table, ntohl(dst_addr.s_addr));

    // no match
    if (rt_index == 0) {
        return 1;
    }

    struct route *item = table[rt_index];

    memcpy(&nexthop_info->host, &item->nexthop->host, sizeof(struct remote_host_t));
    nexthop_info->prefix_len = item->prefix_len;

    return 0;
}

int delete_route(struct in_addr dst_addr, uint32_t prefix_len) {
    rt_remove(routing_table, ntohl(dst_addr.s_addr), prefix_len);
    return 0;
}
