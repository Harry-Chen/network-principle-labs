#ifndef __RT_H__
#define __RT_H__

#include <stdint.h>

// You should use host byte order for all ips passed to these functions.
// Insertion and removal will not fail in any cases.
// If lookup fails, 0 will be returned.

typedef void *routing_table_t;

extern routing_table_t rt_init();
extern void rt_insert(routing_table_t table, uint32_t ip, uint32_t prefix, uint32_t index);
extern void rt_remove(routing_table_t table, uint32_t ip, uint32_t prefix);
extern uint32_t rt_lookup(routing_table_t table, uint32_t ip);

#endif // __RT_H__