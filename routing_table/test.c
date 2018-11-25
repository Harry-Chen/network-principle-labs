#include <stdio.h>
#include <assert.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "rt.h"

#define IP(a) (ntohl(inet_addr(a)))

int main() {

    routing_table_t table = rt_init();
    assert(rt_init != NULL);

    // insert some routes
    rt_insert(table, IP("10.0.0.0"), 8, 1);
    rt_insert(table, IP("100.64.0.0"), 24, 2);
    rt_insert(table, IP("100.64.1.0"), 24, 3);
    rt_insert(table, IP("100.64.0.0"), 10, 4);

    uint32_t result;

    // find some existed routes
    result = rt_lookup(table, IP("10.10.10.10"));
    assert(result == 1);

    result = rt_lookup(table, IP("100.100.100.100"));
    assert(result == 4);

    result = rt_lookup(table, IP("100.64.0.100"));
    assert(result == 2);

    // exact match found
    result = rt_match(table, IP("100.64.0.0"), 24, 1);
    assert(result == 2);

    result = rt_match(table, IP("100.64.0.0"), 10, 1);
    assert(result == 4);

    // exact match not found
    result = rt_match(table, IP("100.64.3.0"), 24, 1);
    assert(result == 0);

    // default: not found
    result = rt_lookup(table, IP("200.200.200.200"));
    assert(result == 0);

    // default: should be found now
    rt_insert(table, IP("0.0.0.0"), 0, 5);
    result = rt_lookup(table, IP("200.200.200.200"));
    assert(result == 5);

    // default: not found again
    rt_remove(table, IP("0.0.0.0"), 0);
    result = rt_lookup(table, IP("200.200.200.200"));
    assert(result == 0);

    // test iteration
    result = 0;
    while ((result = rt_iterate(table, result)) != -1) {}
    assert(result == -1);

    rt_cleanup(table);

    printf("Routing table test passed!\n");

    return 0;
}