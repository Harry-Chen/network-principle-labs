#ifndef __CHECK__
#define __CHECK__

#include <stdint.h>
#include <netinet/ip.h>


uint16_t calculate_check_sum(struct ip* ip_header);

#endif
