#include "recv_route.h"
#include "routing_table.h"

#include "common.h"

extern bool should_exit;

static int static_route_get(int sock_fd, struct selfroute *selfrt) {

	int ret;
    int conn_fd;

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
        send(conn_fd, "Route change received.\n", 6, 0);
        ret = 0;
    }

    close(conn_fd);
    return ret;

}

// thread to receive routing table change
void *receive_rt_change(void *arg) {

    int sock_fd;
	struct sockaddr_in server_addr;
    
	if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		fprintf(stderr, "Error opening TCP socket: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	bzero(&server_addr, sizeof(struct sockaddr_in));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	server_addr.sin_port = htons(800);

	if (bind(sock_fd, (struct sockaddr *)(&server_addr), sizeof(struct sockaddr)) == -1) {
		fprintf(stderr, "Error binding to local address: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

    if (listen(sock_fd, 5) == -1) {
		fprintf(stderr, "Listen error:%s\n\a", strerror(errno));
		exit(EXIT_FAILURE);
	}

    int st = 0;
    struct selfroute selfrt;
    char ifname[IF_NAMESIZE];
    char ip_addr_next[INET_ADDRSTRLEN], ip_addr_prefix[INET_ADDRSTRLEN];

    while (!should_exit) {
        st = static_route_get(sock_fd, &selfrt);
        if (st == 0) {
            if_indextoname(selfrt.ifindex, ifname);
            inet_ntop(AF_INET, &(selfrt.nexthop.s_addr), ip_addr_next, INET_ADDRSTRLEN);
            inet_ntop(AF_INET, &(selfrt.prefix.s_addr), ip_addr_prefix, INET_ADDRSTRLEN);
            printf("cmd: %d, prefix: %s/%d, next hop: %s, interface: %d(%s)\n", selfrt.cmdnum, ip_addr_prefix, selfrt.prefixlen, ip_addr_next, selfrt.ifindex, ifname);
            if (selfrt.cmdnum == 24) {
                // insert to routing table
                insert_route(selfrt.prefix, selfrt.prefixlen, ifname, selfrt.ifindex, selfrt.nexthop);
                printf("Route inserted to table.\n");
            } else if (selfrt.cmdnum == 25) {
                // delete from routing table
                delete_route(selfrt.prefix, selfrt.prefixlen);
                printf("Route deleted from table.\n");
            }
        }
    }

    close(sock_fd);
    return NULL;
}