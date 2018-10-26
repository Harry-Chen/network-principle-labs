#ifndef __ARP__
#define __ARP__

#include <linux/if_ether.h>

typedef unsigned char macaddr_t[ETH_ALEN];

int arp_get_mac(int sock_fd, macaddr_t mac, char *ifname, char *ip_str);

#endif
