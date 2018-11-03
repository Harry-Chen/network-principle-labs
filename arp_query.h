#ifndef __ARP__
#define __ARP__

#include "common.h"

typedef unsigned char macaddr_t[ETH_ALEN];

int arp_get_mac(int sock_fd, macaddr_t mac, char *ifname, in_addr_t ip_addr);

#endif
