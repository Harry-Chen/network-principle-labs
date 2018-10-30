#include "recv_route.h"

#include "common.h"

int static_route_get(struct selfroute *selfrt) {
    int sock_fd, conn_fd;
	struct sockaddr_in server_addr;
	int ret;

	if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		fprintf(stderr, "Error opening TCP socket: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	bzero(&server_addr, sizeof(struct sockaddr_in));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(inet_addr("127.0.0.1"));
	server_addr.sin_port = htons(800);

	if (bind(sock_fd, (struct sockaddr *)(&server_addr), sizeof(struct sockaddr)) == -1) {
		fprintf(stderr, "Error binding to local address: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

    if (listen(sock_fd, 5) == -1) {
		fprintf(stderr, "Listen error:%s\n\a", strerror(errno));
		exit(EXIT_FAILURE);
	}

    if ((conn_fd = accept(sock_fd, (struct sockaddr *)NULL, NULL)) == -1) {
        fprintf(stderr, "Accept socket error: %s\n\a", strerror(errno));
        exit(EXIT_FAILURE);
    }

    ret = recv(conn_fd, selfrt, sizeof(struct selfroute), 0);

    if (ret < sizeof(struct selfroute)) {
        printf("Received message is not of type struct selfroute, ignored.\n");
        ret = -1;
    } else {
        printf("Received message from Quagga:");
        send(conn_fd, "Route change received.", 6, 0);
        ret = 0;
    }

    close(conn_fd);
    close(sock_fd);
    return ret;

}
