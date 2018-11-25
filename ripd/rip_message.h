#ifndef __RIP_MESSAGE_H__
#define __RIP_MESSAGE_H__

#include "common.h"

void send_all_routes(in_addr_t dest);

void send_rip_request();

void handle_rip_messages();

#endif