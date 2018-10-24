#ifndef __SEND__
#define __SEND__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <net/ethernet.h>
#include <net/if_arp.h>
#include <net/if.h>
#include <netpacket/packet.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip.h>

/* 以太网帧首部长度 */
#define ETHER_HEADER_LEN sizeof(struct ether_header)
/* 整个ip首部长度 */
#define IP_HEADER_LEN sizeof(struct ip)
/* 以太网 + 整个ip首部长度*/
#define ETHER_IP_LEN ETHER_HEADER_LEN + IP_HEADER_LEN
/* IP地址长度 */
#define IP_ADDR_LEN 4
 
struct ip *fill_ip_packet(struct ip *ip_packet,unsigned short checksum);
void ip_transmit(struct ip *ip_packet,unsigned short checksum,char *name,unsigned char *nextmac,char *bufdata,int datalength);

#endif

