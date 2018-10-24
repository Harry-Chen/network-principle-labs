#ifndef __RT_H__
#define __RT_H__

#include <stdint.h>

extern void *rt_init();
extern void rt_insert(void *table, uint32_t ip, uint32_t prefix, uint32_t index);
extern void rt_remove(void *table, uint32_t ip, uint32_t prefix);
extern uint32_t rt_lookup(void *table, uint32_t ip);

#endif