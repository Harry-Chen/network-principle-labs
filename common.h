#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <netinet/if_ether.h>
#include <netpacket/packet.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <ifaddrs.h>

#define NEXTHOP_ONLINK UINT32_MAX
#define NEXTHOP_SELF 0

#endif