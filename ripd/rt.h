#ifndef __RT_H__
#define __RT_H__

#include <stdint.h>

// You should use host byte order for all ips passed to these functions.
// Insertion and removal will not fail in any cases.
// If lookup fails, 0 will be returned.

#ifdef __cplusplus
extern "C" {
#endif

extern void rt_init();
extern void rt_insert(uint32_t ip, uint32_t prefix, uint32_t index);
extern void rt_remove(uint32_t ip, uint32_t prefix);
extern uint32_t rt_match(uint32_t ip, uint32_t prefix, uint32_t exact);
extern uint32_t rt_lookup(uint32_t ip);
extern void rt_cleanup();
extern uint32_t rt_iterate(uint32_t nextOf);

#ifdef __cplusplus
}
#endif

#endif // __RT_H__