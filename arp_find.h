#ifndef __ARP__
#define __ARP__

struct arpmac {
    unsigned char *mac;
    unsigned int index;
};

int arp_get(struct arpmac *srcmac, char *ifname, char *ip_str);

#endif
