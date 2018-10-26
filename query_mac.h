#ifndef __ARP__
#define __ARP__

#include <linux/if_ether.h>

typedef unsigned char macaddr_t[ETH_ALEN];

int arp_get_mac(macaddr_t mac, char *ifname, char *ip_str);
int if_get_mac(macaddr_t mac, char *ifname);

#endif
