#ifndef __RT_H__
#define __RT_H__

#include <stdint.h>

// You should use host byte order for all ips passed to these functions.
// Insertion and removal will not fail in any cases.
// If lookup fails, 0 will be returned.

extern void *rt_init();
extern void rt_insert(void *table, uint32_t ip, uint32_t prefix, uint32_t index);
extern void rt_remove(void *table, uint32_t ip, uint32_t prefix);
extern uint32_t rt_lookup(void *table, uint32_t ip);

#endif // __RT_H__