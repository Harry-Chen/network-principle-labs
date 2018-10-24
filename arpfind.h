#ifndef __ARP__
#define __ARP__
#include <stdio.h>   
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>  
#include <sys/ioctl.h>  
#include <net/if_arp.h>  
#include <string.h> 

struct arpmac
{
    unsigned char * mac;
    unsigned int index;
};


int arpGet(struct arpmac *srcmac,char *ifname, char *ipStr);

#endif 
