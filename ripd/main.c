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

	// install signal handler
	if (signal(SIGINT, signal_handler) == SIG_ERR) {
        fprintf(stderr, "Error registering signal handler!\n");
        exit(EXIT_FAILURE);
    } else {
        printf("Starting RIP daemon...\n");
    }

	// start thread to send updating messages every 30 seconds
	pthread_t tid_update, tid_recv_response;
    int pd_update, pd_recv_response;
    if ((pd_update = pthread_create(&tid_update, NULL, send_update_messages, NULL)) < 0) {
        fprintf(stderr, "Error creating thread: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    } else {
        printf("Thread started to send rip routing table every %d seconds.\n", RIP_UPDATE_INTERVAL);
    }

	// send initial reuqest to get rip table from other hosts
	send_request_messages();

	// receive reponse messages
	if ((pd_recv_response = pthread_create(&tid_recv_response, NULL, receive_and_handle_rip_messages, NULL)) < 0) {
        fprintf(stderr, "Error creating thread: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    } else {
        printf("Thread started to receive response messages.\n");
    }

	while (!should_exit) {}

	exit(0);
}

