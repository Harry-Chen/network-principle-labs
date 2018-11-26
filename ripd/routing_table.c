#include <assert.h>
#include <pthread.h>

#include "routing_table.h"
#include "local_route.h"
#include "../routing_table/rt.h"

static routing_table_t routing_table;
static int CMD_ADD = 24;
static int CMD_DEL = 25;

#define MAX_TABLE_SIZE (1 << 20)
static TRtEntry *table[MAX_TABLE_SIZE];
static int table_size = 1;

static pthread_rwlock_t rwlock;
static pthread_mutex_t mutex;

void init_route() { 
    pthread_rwlock_init(&rwlock, NULL);
    pthread_mutex_init(&mutex, NULL);
    routing_table = rt_init(); 
}

void insert_route_local(TRtEntry *entry) {
    TRtEntry *item = (TRtEntry *)malloc(sizeof(TRtEntry));
    memcpy(item, entry, sizeof(TRtEntry));
    item->stIpPrefix.s_addr = htonl(ntohl(item->stIpPrefix.s_addr) & PREFIX_DEC2BIN(item->uiPrefixLen));

    pthread_rwlock_wrlock(&rwlock);
    table[table_size] = item;
    rt_insert(routing_table, ntohl(item->stIpPrefix.s_addr), item->uiPrefixLen, table_size);
    table_size++;
    pthread_rwlock_unlock(&rwlock);

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

    pthread_rwlock_wrlock(&rwlock);
    table[table_size] = item;
    rt_insert(routing_table, ntohl(item->stIpPrefix.s_addr), item->uiPrefixLen,
              table_size);
    table_size++;
    pthread_rwlock_unlock(&rwlock);
    
    notify_forwarder(item, CMD_ADD);
}

TRtEntry *lookup_route_longest(struct in_addr dst_addr) {
    pthread_rwlock_rdlock(&rwlock);
    uint32_t rt_index = rt_lookup(routing_table, ntohl(dst_addr.s_addr));
    pthread_rwlock_unlock(&rwlock);
    return rt_index == 0 ? NULL : table[rt_index];
}

TRtEntry *lookup_route_exact(struct in_addr dst_addr, uint32_t prefix) {
    pthread_rwlock_rdlock(&rwlock);
    uint32_t rt_index =
        rt_match(routing_table, ntohl(dst_addr.s_addr), prefix, 1);
    pthread_rwlock_unlock(&rwlock);
    return rt_index == 0 ? NULL : table[rt_index];
}

void delete_route_rip(TRtEntry *entry) {
    pthread_rwlock_wrlock(&rwlock);
    rt_remove(routing_table, ntohl(entry->stIpPrefix.s_addr),
              entry->uiPrefixLen);
    pthread_rwlock_unlock(&rwlock);

    notify_forwarder(entry, CMD_DEL);
}

int fill_rip_packet(TRipEntry *rip_entry, struct in_addr nexthop) {
    int size = 0;
    int index = 0;

    pthread_rwlock_rdlock(&rwlock);
    while ((index = rt_iterate(routing_table, index)) != -1) {
        TRtEntry *rt_entry = table[index];
        
        // retrive the 'real' next hop for the route entry, and find its outbound interface
        TRtEntry *local_route = lookup_route_longest(rt_entry->stNexthop);
        assert(local_route != NULL);

        // if it is just the interfaces we are sending multicast from
        // either we learn the route from this interface, or it is the on-link route
        uint8_t nexthop_same_interface = get_interface_info(local_route->uiInterfaceIndex)->ip.s_addr == nexthop.s_addr;
        uint8_t onlink_route = rt_entry->stNexthop.s_addr == nexthop.s_addr;
        if (nexthop_same_interface && !onlink_route) {
            // horizontal split: do not send the route back
            continue;
        }

        rip_entry[size].usFamily = htons(AF_INET);
        rip_entry[size].usTag = 0;
        // calculate the actual network ip & prefix
        rip_entry[size].stPrefixLen.s_addr = htonl(PREFIX_DEC2BIN(rt_entry->uiPrefixLen));
        rip_entry[size].stAddr.s_addr = rt_entry->stIpPrefix.s_addr & rip_entry[size].stPrefixLen.s_addr;

        if (is_local_address(rt_entry->stNexthop)) {
            rip_entry[size].stNexthop.s_addr = 0; // according to RFC, means 'self'
        } else {
            rip_entry[size].stNexthop = rt_entry->stNexthop;
        }
        rip_entry[size].uiMetric = htonl(rt_entry->uiMetric);
        ++size;
    }
    pthread_rwlock_unlock(&rwlock);

    return size;
}

void print_all_routes(FILE *f) {
    int index = 0;
    pthread_rwlock_rdlock(&rwlock);
    pthread_mutex_lock(&mutex);
    while ((index = rt_iterate(routing_table, index)) != -1) {
        TRtEntry *entry = table[index];
        fprintf(f, "[Current Route] %s/%d ", inet_ntoa(entry->stIpPrefix), entry->uiPrefixLen);
        fprintf(f, "via %s metric %d\n", inet_ntoa(entry->stNexthop), entry->uiMetric);
    }
    pthread_mutex_unlock(&mutex);
    pthread_rwlock_unlock(&rwlock);
}