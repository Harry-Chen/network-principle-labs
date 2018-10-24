#ifndef __ARP__
#define __ARP__
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>

struct arpmac {
    unsigned char *mac;
    unsigned int index;
};

int arpGet(struct arpmac *srcmac, char *ifname, char *ipStr);

#endif
