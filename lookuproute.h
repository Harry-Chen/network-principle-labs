#ifndef __FIND__
#define __FIND__
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include <arpa/inet.h>


struct route
{
    struct route *next;
    struct in_addr ip4prefix;
	unsigned int prefixlen;
    struct nexthop *nexthop;
};

struct nexthop
{
   struct nexthop *next;
   char *ifname;
   unsigned int ifindex;//zlw ifindex2ifname()获取出接�?   // Nexthop address 
   struct in_addr nexthopaddr;
};

struct nextaddr
{
   char *ifname;
   struct in_addr ipv4addr;
   unsigned int prefixl;
};

struct route *route_table; 
int insert_route(unsigned long  ip4prefix,unsigned int prefixlen,char *ifname,unsigned int ifindex,unsigned long  nexthopaddr);
int lookup_route(struct in_addr dstaddr,struct nextaddr *nexthopinfo);
int delete_route(struct in_addr dstaddr,unsigned int prefixlen);

#endif
