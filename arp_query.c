#include "arp_query.h"

int arp_get_mac(int sock_fd, macaddr_t mac, char *if_name, in_addr_t ip_addr) { 

    struct arpreq arp_req;
    struct sockaddr_in *sin;

    sin = (struct sockaddr_in *)&(arp_req.arp_pa);

    memset(&arp_req, 0, sizeof(arp_req));
    sin->sin_family = AF_INET;
    sin->sin_addr.s_addr = ip_addr;
    strncpy(arp_req.arp_dev, if_name, IF_NAMESIZE - 1);

    int ret;

    if ((ret = ioctl(sock_fd, SIOCGARP, &arp_req)) < 0)  {
        return ret;
    }

    if (arp_req.arp_flags & ATF_COM) {
        memcpy(mac, (unsigned char *)arp_req.arp_ha.sa_data, sizeof(macaddr_t));
        return 0;
    } else {
        return 1;
    }

}