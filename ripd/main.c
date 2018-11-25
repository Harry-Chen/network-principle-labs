#include <pthread.h>
#include <signal.h>

#include "common.h"
#include "routing_table.h"
#include "local_route.h"
#include "rip_message.h"

int should_exit = 0;

void signal_handler(int signo) {
    if (signo == SIGINT) {
        printf("Received SIGINT, exiting...\n");
        should_exit = 1;
    }
}

int main(int argc,char* argv[]) {
	
	// init rip routing table
	init_route();

	// find all local interfaces, and add link-scoped routes to rip routing table
	init_local_interfaces();

	// start thread to send updating messages every 30 seconds
	pthread_t tid_update, tid_recv_response, tid_recv_multicast;
    int pd_update, pd_recv_response, pd_recv_multicast;
    if ((pd_update = pthread_create(&tid_update, NULL, send_update_messages, NULL)) < 0) {
        fprintf(stderr, "Error creating thread: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    } else {
        printf("Thread started to send rip routing table every 30 seconds.\n");
    }

	// send initial reuqest to get rip table from other hosts
	send_request_messages();

	// receive unicast reponse messages
	uint8_t multicast = 0;
	if ((pd_recv_response = pthread_create(&tid_recv_response, NULL, receive_and_handle_rip_messages, &multicast)) < 0) {
        fprintf(stderr, "Error creating thread: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    } else {
        printf("Thread started to receive response messages.\n");
    }

	// receive multicast response messages
	multicast = 1;
	if ((pd_recv_multicast = pthread_create(&tid_recv_multicast, NULL, receive_and_handle_rip_messages, &multicast)) < 0) {
        fprintf(stderr, "Error creating thread: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    } else {
        printf("Thread started to receive multicast messages.\n");
    }

	while (!should_exit) {}

	return 0;
}

