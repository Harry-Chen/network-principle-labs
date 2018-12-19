#ifndef __MYRIP_H
#define __MYRIP_H
#include <stdio.h>  
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h> 
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <net/if.h>
#include <pthread.h>
#include <errno.h>
#include <ifaddrs.h>
#include <arpa/inet.h>

#define RIP_VERSION    	2
#define RIP_REQUEST    	1
#define RIP_RESPONSE   	2
#define RIP_INFINITY  	16
#define RIP_PORT		520
#define RIP_PACKET_HEAD	4
#define RIP_MAX_PACKET  504
#define RIP_MAX_ENTRY   25
#define ROUTE_MAX_ENTRY 256
#define RIP_GROUP		"224.0.0.9"
#define RIP_HEADER_LEN  4
#define RIP_UPDATE_INTERVAL 5

#define RIP_CHECK_OK    1
#define RIP_CHECK_FAIL  0
#define AddRoute        24
#define DelRoute        25

extern int should_exit;

typedef struct RipEntry
{
	uint16_t usFamily;
	uint16_t usTag;
	struct in_addr stAddr;
	struct in_addr stPrefixLen;
	struct in_addr stNexthop;
	uint32_t uiMetric;
} TRipEntry;

typedef struct RipPacket
{
	uint8_t ucCommand;
	uint8_t ucVersion;
	uint16_t usZero; 
	TRipEntry RipEntries[RIP_MAX_ENTRY];
} TRipPkt;


typedef struct RouteEntry
{
	struct in_addr stIpPrefix; 
	uint32_t uiPrefixLen;
	struct in_addr stNexthop;
	uint32_t uiMetric;
	uint32_t uiInterfaceIndex;
} TRtEntry;

typedef struct SockRoute
{
	uint32_t uiPrefixLen;
	struct in_addr stIpPrefix;
	uint32_t uiIfindex;
	struct in_addr stNexthop;
	uint32_t uiCmd;
	// for compatibility purpose
	char cIfName[10];
} TSockRoute;

#define PREFIX_BIN2DEC(bin) (32 - __builtin_ctz((bin)))
#define PREFIX_DEC2BIN(hex) (((~0) >> (32 - (hex))) << (32 - (hex)))

#endif

