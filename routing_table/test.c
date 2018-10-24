#include <stdio.h>

#include "rt.h"


int main() {
    void *table = rt_init();
    printf("Routing table test result: %d\n", rt_test(table));
    return 0;
}