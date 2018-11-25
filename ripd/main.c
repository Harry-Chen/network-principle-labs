#include <pthread.h>

#include "common.h"
#include "routing_table.h"
#include "local_route.h"
#include "rip_message.h"

void *send_update_messages(void *arg) {
	while (1) {
		send_all_routes(inet_addr(RIP_GROUP));
		sleep(30);
	}
}

int main(int argc,char* argv[]) {
	
	// init rip routing table
	init_route();

	// find all local interfaces, and add link-scoped routes to rip routing table
	init_local_interfaces();

	// start thread to send updating messages every 30 seconds
	pthread_t tid;
    int pd;
    if ((pd = pthread_create(&tid, NULL, send_update_messages, NULL)) < 0) {
        fprintf(stderr, "Error creating thread.\n");
        exit(EXIT_FAILURE);
    } else {
        printf("Thread started to send rip routing table every 30 seconds.\n");
    }

	// send initial reuqest to get rip table from other hosts
	send_rip_request();

	// infinitely receive rip messages and handle it
	handle_rip_messages();

	sleep(20);

	return 0;
}

