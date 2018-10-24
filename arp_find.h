#ifndef __ARP__
#define __ARP__

typedef unsigned char macaddr_t[6];

int arp_get(macaddr_t mac, char *ifname, char *ip_str);

#endif
