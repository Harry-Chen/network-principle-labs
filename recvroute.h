#ifndef __RECVROUTE__
#define __RECVROUTE__
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<netinet/ip.h>
#include<netinet/ip_icmp.h>
#include<sys/time.h>
#include<linux/if_ether.h>
#include<arpa/inet.h>
#include<net/route.h>
 
struct selfroute
{
     u_char prefixlen;
     struct in_addr prefix;
     unsigned int ifindex;
     struct in_addr nexthop;
	 unsigned int cmdnum;
	 char ifname[10];
}buf2;

int static_route_get(struct selfroute *selfrt);
#endif
