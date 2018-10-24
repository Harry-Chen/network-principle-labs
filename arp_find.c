#include "arp_find.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <arpa/inet.h>
#include <net/if_arp.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>

int arp_get(macaddr_t mac, char *if_name, char *ip_str) { 

    struct arpreq arp_req;
    struct sockaddr_in *sin;

    sin = (struct sockaddr_in *)&(arp_req.arp_pa);

    memset(&arp_req, 0, sizeof(arp_req));
    sin->sin_family = AF_INET;
    inet_pton(AF_INET, ip_str, &(sin->sin_addr));
    strncpy(arp_req.arp_dev, if_name, IFNAMSIZ-1);

    int sfd = socket(AF_INET, SOCK_DGRAM, 0);

    int ret = ioctl(sfd, SIOCGARP, &arp_req);
    if (ret < 0) {
        fprintf(stderr, "Get ARP entry failed : %s\n", strerror(errno));
        exit(2);
    }

    if (arp_req.arp_flags & ATF_COM) {
        memcpy(mac, (unsigned char *)arp_req.arp_ha.sa_data, sizeof(macaddr_t));
        printf("MAC Address: %02x:%02x:%02x:%02x:%02x:%02x\n",
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        return 0;
    } else {
        printf("MAC Address not in the ARP cache.\n");
        return 1;
    }
    
}
