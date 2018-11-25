#include "routing_table.h"
#include "../routing_table/rt.h"

#include "common.h"

static routing_table_t routing_table;

#define MAX_TABLE_SIZE (1<<20)
static struct route *table[MAX_TABLE_SIZE];
static int table_size = 1;

static struct in_addr last_query;
static int dirty;
static int last_result;

struct in_addr NEXTHOP_ONLINK =  {
    .s_addr = UINT32_MAX
};

struct in_addr NEXTHOP_SELF =  {
    .s_addr = 0
};

void init_route() {
    dirty = 1;
    routing_table = rt_init();
}

int insert_route(struct in_addr ip_prefix, uint32_t prefix_len, char *if_name,
                 uint32_t if_index, struct in_addr nexthop_addr) {

    dirty = 1;

    struct route* item = (struct route*) malloc(sizeof(struct route));
    item->ip_prefix = ip_prefix;
    item->prefix_len = prefix_len;
    item->nexthop = (struct nexthop*) malloc(sizeof(struct nexthop));
    item->nexthop->host.if_name = (char *) malloc(strlen(if_name));
    strcpy(item->nexthop->host.if_name, if_name);
    item->nexthop->host.if_index = if_index;
    item->nexthop->host.addr = nexthop_addr;

    table[table_size] = item;
    rt_insert(routing_table, ntohl(ip_prefix.s_addr), prefix_len, table_size);
    table_size++;

    return 0;
}

int lookup_route(struct in_addr dst_addr, struct nextaddr *nexthop_info) {

    if (dirty == 0 && dst_addr.s_addr == last_query.s_addr) {
        // direct hit
        return last_result;
    }

    uint32_t rt_index = rt_lookup(routing_table, ntohl(dst_addr.s_addr));
    last_query.s_addr = dst_addr.s_addr;

    // no match
    if (rt_index == 0) {
        dirty = 0;
        return (last_result = 1);
    }

    struct route *item = table[rt_index];

    memcpy(&nexthop_info->host, &item->nexthop->host, sizeof(struct remote_host_t));
    nexthop_info->prefix_len = item->prefix_len;

    dirty = 0;
    return (last_result = 0);
}

int delete_route(struct in_addr dst_addr, uint32_t prefix_len) {
    dirty = 1;
    rt_remove(routing_table, ntohl(dst_addr.s_addr), prefix_len);
    return 0;
}
