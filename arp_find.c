#include "arp_find.h"

#include <arpa/inet.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>

int arp_get(struct arpmac *srcmac, char *ifname, char *ip_str) { 
    return 0; 
}
