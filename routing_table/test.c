#include <stdio.h>
#include <assert.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "rt.h"

#define IP(a) (ntohl(inet_addr(a)))

int main() {

    void *table = rt_init();

    rt_insert(table, IP("10.0.0.0"), 8, 1);
    rt_insert(table, IP("100.64.0.0"), 24, 2);
    rt_insert(table, IP("100.64.1.0"), 24, 3);
    rt_insert(table, IP("100.64.0.0"), 10, 4);

    uint32_t result;

    result = rt_lookup(table, IP("10.10.10.10"));
    assert(result == 1);

    result = rt_lookup(table, IP("100.100.100.100"));
    assert(result == 4);

    result = rt_lookup(table, IP("100.64.0.100"));
    assert(result == 2);

    // default
    result = rt_lookup(table, IP("200.200.200.200"));
    assert(result == 0);


    printf("Routing table test passed!\n");

    return 0;
}