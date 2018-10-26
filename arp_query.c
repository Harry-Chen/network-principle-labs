#include "arp_query.h"

#include "common.h"

int arp_get_mac(macaddr_t mac, char *if_name, char *ip_str) { 

    struct arpreq arp_req;
    struct sockaddr_in *sin;

    sin = (struct sockaddr_in *)&(arp_req.arp_pa);

    memset(&arp_req, 0, sizeof(arp_req));
    sin->sin_family = AF_INET;
    inet_pton(AF_INET, ip_str, &(sin->sin_addr));
    strncpy(arp_req.arp_dev, if_name, IF_NAMESIZE - 1);

    int sfd = socket(AF_INET, SOCK_DGRAM, 0);

    int ret = ioctl(sfd, SIOCGARP, &arp_req);
    if (ret < 0) {
        fprintf(stderr, "Get ARP entry failed for %s @%s : %s\n", ip_str, if_name, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (arp_req.arp_flags & ATF_COM) {
        memcpy(mac, (unsigned char *)arp_req.arp_ha.sa_data, sizeof(macaddr_t));
        ret = 0;
    } else {
        ret = 1;
    }
    
    close(sfd);
    return ret;

}