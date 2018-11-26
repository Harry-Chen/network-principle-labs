#include <assert.h>

#include "routing_table.h"
#include "../routing_table/rt.h"

static routing_table_t routing_table;
static int CMD_ADD = 24;
static int CMD_DEL = 25;

#define MAX_TABLE_SIZE (1 << 20)
static TRtEntry *table[MAX_TABLE_SIZE];
static int table_size = 1;

void init_route() { routing_table = rt_init(); }

void insert_route_local(TRtEntry *entry) {
    TRtEntry *item = (TRtEntry *)malloc(sizeof(TRtEntry));
    memcpy(item, entry, sizeof(TRtEntry));
    uint32_t ip_h = ntohl(item->stIpPrefix.s_addr);
    ip_h &= PREFIX_DEC2BIN(item->uiPrefixLen);;
    table[table_size] = item;
    rt_insert(routing_table, ip_h, item->uiPrefixLen, table_size);
    table_size++;
}

static void notify_forwarder(TRtEntry *entry, uint32_t cmd) {
    
    int sendfd;

    struct sockaddr_in dst_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(800)
    };
    dst_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    TSockRoute route;
    route.uiPrefixLen = entry->uiPrefixLen;
    route.stIpPrefix = entry->stIpPrefix;
    route.uiIfindex = entry->uiInterfaceIndex;
    route.stNexthop = entry->stNexthop;
    route.uiCmd = cmd;

    if ((sendfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf(stderr, "[Notify] Error opening TCP socket: %s\n", strerror(errno));
        return;
    }

    if (connect(sendfd, (struct sockaddr *)&dst_addr, sizeof(dst_addr)) < 0) {
        fprintf(stderr, "[Notify] Connecting to forwarder failed: %s\n", strerror(errno));
        return;
    }

    if (send(sendfd, &route, sizeof(route), 0) < 0) {
        fprintf(stderr, "[Notify] Sending route item failed: %s\n", strerror(errno));
        return;
    } else {
        fprintf(stderr, "[Notify] Successfully sent route item to forwarder\n");
    }

    close(sendfd);
}

void insert_route_rip(TRipEntry *entry) {
    TRtEntry *item = (TRtEntry *)malloc(sizeof(TRtEntry));
    item->stIpPrefix = entry->stAddr;
    item->uiPrefixLen = PREFIX_BIN2DEC(ntohl(entry->stPrefixLen.s_addr));
    item->uiMetric = ntohl(entry->uiMetric) + 1;
    item->stNexthop = entry->stNexthop;
    TRtEntry *local_route = lookup_route_longest(item->stNexthop);
    assert(local_route != NULL);
    item->uiInterfaceIndex = local_route->uiInterfaceIndex;
    table[table_size] = item;
    rt_insert(routing_table, ntohl(item->stIpPrefix.s_addr), item->uiPrefixLen,
              table_size);
    table_size++;
    notify_forwarder(item, CMD_ADD);
}

TRtEntry *lookup_route_longest(struct in_addr dst_addr) {
    uint32_t rt_index = rt_lookup(routing_table, ntohl(dst_addr.s_addr));
    return rt_index == 0 ? NULL : table[rt_index];
}

TRtEntry *lookup_route_exact(struct in_addr dst_addr, uint32_t prefix) {
    uint32_t rt_index =
        rt_match(routing_table, ntohl(dst_addr.s_addr), prefix, 1);
    return rt_index == 0 ? NULL : table[rt_index];
}

void delete_route_rip(TRtEntry *entry) {
    rt_remove(routing_table, ntohl(entry->stIpPrefix.s_addr),
              entry->uiPrefixLen);
    notify_forwarder(entry, CMD_DEL);
}

int fill_rip_packet(TRipEntry *rip_entry, struct in_addr iface_addr) {
    int size = 0;
    int index = 0;
    while ((index = rt_iterate(routing_table, index)) != -1) {
        TRtEntry *rt_entry = table[index];
        rip_entry[size].usFamily = htons(AF_INET);
        rip_entry[size].usTag = 0;
        // calculate the actual network ip & prefix
        rip_entry[size].stPrefixLen.s_addr = htonl(PREFIX_DEC2BIN(rt_entry->uiPrefixLen));
        rip_entry[size].stAddr.s_addr = rt_entry->stIpPrefix.s_addr & rip_entry[size].stPrefixLen.s_addr;
        rip_entry[size].stNexthop = iface_addr;
        rip_entry[size].uiMetric = htonl(rt_entry->uiMetric);
        ++size;
    }

    return size;
}

void print_all_routes(FILE *f) {
    int index = 0;
    while ((index = rt_iterate(routing_table, index)) != -1) {
        TRtEntry *entry = table[index];
        fprintf(f, "[Current Route] %s/%d ", inet_ntoa(entry->stIpPrefix), entry->uiPrefixLen);
        fprintf(f, "via %s metric %d\n", inet_ntoa(entry->stNexthop), entry->uiMetric);
    }
}