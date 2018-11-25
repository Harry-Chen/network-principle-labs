#ifndef __RIP_MESSAGE_H__
#define __RIP_MESSAGE_H__

#include "common.h"

void send_request_messages();

void *send_update_messages(void *args);

void *receive_and_handle_rip_messages(void *args);

#endif