#include "rip_message.h"
#include "routing_table.h"
#include "local_route.h"

static TRipPkt packet_request, packet_response, packet_update;
static int UPDATE = 0;
static int REQUEST = 1;

static int establish_rip_fd(in_addr_t source, in_addr_t dest, uint8_t do_connect) {
    int fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (fd < 0) {
        fprintf(stderr, "Error opening socket: %s\n", strerror(errno));
        return -1;
    }
    
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0) {
        fprintf(stderr, "Set SO_REUSEADDR on socket failed: %s\n", strerror(errno));
        return -1;
    }

    if (!do_connect) {
        // join multicast groups to listen
        struct ip_mreq_source mreq;
        mreq.imr_multiaddr.s_addr = inet_addr(RIP_GROUP);
        
        for (int i = 0; i < MAX_IF; ++i) {
            if_info_t *iface = get_interface_info(i);
            if (iface->name[0] == '\0' || !iface->multicast) continue; // empty interface or cannot multicast
            mreq.imr_interface = iface->ip;
            mreq.imr_sourceaddr = iface->ip;
            // ip_mreq has just two members of ip_mreq_source
            if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(struct ip_mreq)) < 0) {
                fprintf(stderr, "Join multicast group failed: %s\n", strerror(errno));
                return -1;
            }
            if (setsockopt(fd, IPPROTO_IP, IP_BLOCK_SOURCE, &mreq, sizeof(mreq)) < 0) {
                fprintf(stderr, "Block multicast from local interface failed: %s\n", strerror(errno));
                return -1;
            }
        }
    }

    struct sockaddr_in local;
    local.sin_family = AF_INET;
    local.sin_port = htons(RIP_PORT);
    local.sin_addr.s_addr = source;

    if (bind(fd, (struct sockaddr *)&local, sizeof(local)) < 0) {
        fprintf(stderr, "Bind to local RIP port failed: %s\n", strerror(errno));
        return -1;
    }

    if (do_connect) { // connect to remote host
        struct sockaddr_in remote;
        remote.sin_family = AF_INET;
        remote.sin_port = htons(RIP_PORT);
        remote.sin_addr.s_addr = dest;
        if (connect(fd, (struct sockaddr *)&remote, sizeof(remote)) < 0) {
            fprintf(stderr, "Connect to remote RIP port failed: %s\n", strerror(errno));
            return -1;
        }
    }

    return fd;
}

static void fill_and_send_multicast_packet(int mode) {

    int length = RIP_HEADER_LEN;
    int payload_size = 0;
    TRipPkt *packet = NULL;

    if (mode == UPDATE) {
        packet = &packet_update;
        struct in_addr placeholder = {
            .s_addr = 0
        };
        payload_size = fill_rip_packet(packet->RipEntries, placeholder);
        length += sizeof(TRipEntry) * payload_size;
    } else if (mode == REQUEST) {
        packet = &packet_request;
        length += sizeof(TRipEntry);
    }

    for (int i = 0; i < MAX_IF; ++i) {
        if_info_t *iface = get_interface_info(i);
        if (iface->name[0] == '\0' || !iface->multicast) continue; // empty interface or cannot multicast

        int fd = establish_rip_fd(iface->ip.s_addr, inet_addr(RIP_GROUP), 1);
        if (fd < 0) continue;

        if (setsockopt(fd, IPPROTO_IP, IP_MULTICAST_IF, &iface->ip, sizeof(struct in_addr)) < 0) {
            fprintf(stderr, "Set IP_MULTICAST_IF failed for %s with IP %s: %s\n", iface->name, inet_ntoa(iface->ip), strerror(errno));
            continue;
        }
        // nexthop should be ip of the interface that is sending packet from
        for (int j = 0; j < payload_size; ++j) {
            packet->RipEntries[j].stNexthop = iface->ip;
        }
        fprintf(stderr, "[Send %s] Multicasting via interface %s with IP %s, size %d...", mode ? "Request" : "Update", iface->name, inet_ntoa(iface->ip), length);
        if (send(fd, packet, length, 0) < 0) {
            fprintf(stderr, "Failed: %s\n", strerror(errno));
        } else {
            fprintf(stderr, "Succeeded!\n");
        }

        close(fd);
    }
}


static void send_all_routes(in_addr_t dest) {

    bzero(&packet_update, sizeof(TRipPkt));
    packet_update.ucCommand = RIP_RESPONSE;
    packet_update.ucVersion = RIP_VERSION;

    printf("Start sending RIP update...\n");
    fill_and_send_multicast_packet(UPDATE);

}


void *send_update_messages(void *args) {
	while (!should_exit) {
		send_all_routes(inet_addr(RIP_GROUP));
		sleep(RIP_UPDATE_INTERVAL);
	}
    return NULL;
}

void send_request_messages() {

    packet_request.ucCommand = RIP_REQUEST;
    packet_request.ucVersion = RIP_VERSION;
    packet_request.RipEntries[0].usFamily = 0;
    packet_request.RipEntries[0].uiMetric = htonl(RIP_INFINITY);
    
    printf("Start sending RIP request...\n");
    fill_and_send_multicast_packet(REQUEST);

}


static void handle_rip_request(struct in_addr src) {
    

    bzero(&packet_response, sizeof(TRipPkt));
    packet_response.ucCommand = RIP_RESPONSE;
    packet_response.ucVersion = RIP_VERSION;
    // find the address to fill in nexthop field of response
    TRtEntry *entry =  lookup_route_longest(src);
    if_info_t *iface = get_interface_info(entry->uiInterfaceIndex);
    int payload_size = fill_rip_packet(packet_response.RipEntries, iface->ip);
    int length = RIP_HEADER_LEN + sizeof(TRipEntry) * payload_size;

    // respond to the host sending request
    int fd = establish_rip_fd(iface->ip.s_addr, src.s_addr, 1);
    if (fd < 0) return;

    printf("[Response to Request] Send via interface %s from %s", iface->name, inet_ntoa(iface->ip));
    printf("to %s, size %d...", inet_ntoa(src), length);

    if (send(fd, &packet_response, length, 0) < 0) {
        fprintf(stderr, "Failed: %s\n", strerror(errno));
    } else {
        fprintf(stderr, "Succeeded!\n");
    }

    close(fd);
}


static void handle_rip_response(TRipEntry *entires, uint32_t size) {
    for (int i = 0; i < size; ++i) {
        TRipEntry *entry = &entires[i];
        uint32_t new_metric = ntohl(entry->uiMetric);
        uint32_t prefix_len = PREFIX_BIN2DEC(ntohl(entry->stPrefixLen.s_addr));

        // inet_ntoa use one same memory for every call!!!!
        printf("[Handle Response: %d] Received route: %s/%d ", i, inet_ntoa(entry->stAddr), prefix_len);
        printf("via %s metric %d\n", inet_ntoa(entry->stNexthop), new_metric);

        TRtEntry *old = lookup_route_exact(entry->stAddr, prefix_len);

        if (old == NULL) { // new item
            if (new_metric + 1 < RIP_INFINITY) {
                insert_route_rip(entry);
                printf("[Handle Response: %d] No existed route to the same network found, inserted to table.\n", i);
            }
        } else { // existing item
            if (entry->stNexthop.s_addr == old->stNexthop.s_addr) { // updating item
                printf("[Handle Response: %d] Found route to the same network and same nexthop, removing the old one.\n", i);
                delete_route_rip(old); // remove the existed item
                if (new_metric + 1 < RIP_INFINITY) {
                    printf("[Handle Response: %d] Inserting the new route", i);
                    insert_route_rip(entry);
                }
            } else { // replacing item
                printf("[Handle Response: %d] Found route to the same network but different nexthop.\n", i);
                if (new_metric + 1 >= RIP_INFINITY) { // remove existed item
                    delete_route_rip(old);
                } else if (new_metric < old->uiMetric) { // replace with the new one
                    printf("[Handle Response: %d] New item has smaller metric, replacing the old one.\n", i);
                    delete_route_rip(old);
                    insert_route_rip(entry);
                }
            }
        }
    }
}


static void handle_rip_message(TRipPkt *message, ssize_t length, struct in_addr src) {

    uint8_t command = message->ucCommand;
    uint8_t version = message->ucVersion;
    TRipEntry *entries = message->RipEntries;
    uint32_t payload_size = (length - RIP_HEADER_LEN) / sizeof(TRipEntry);
    ssize_t remaining_size = length - RIP_HEADER_LEN - payload_size * sizeof(TRipEntry);

    if (version != RIP_VERSION || (command != RIP_REQUEST && command != RIP_RESPONSE) || remaining_size != 0) {
        fprintf(stderr, "[Multicast Handle] Invalid RIP message received and ignored from %s.\n", inet_ntoa(src));
        return;
    }

    switch (command) {
        case RIP_REQUEST:
            if (payload_size != 1 || entries->usFamily != 0 || entries->uiMetric != ntohl(16)) {
                fprintf(stderr, "[Multicast Handle] Received invalid RIP request, will not respond.\n");
                return;
            }
            handle_rip_request(src);
            break;
        case RIP_RESPONSE:
            handle_rip_response(entries, payload_size);
            break;
    }

}


void *receive_and_handle_rip_messages(void *args) {


    // listen on all local interfaces
    int fd = establish_rip_fd(htonl(INADDR_ANY), 0, 0);
    if (fd < 0) return NULL;

    if (setsockopt(fd, IPPROTO_IP, IP_MULTICAST_LOOP, &(int){ 0 }, sizeof(int)) < 0) {
        fprintf(stderr, "Set MULTICAST_LOOP failed: %s\n", strerror(errno));
        return NULL;
    }

    char buffer[1500];

    struct sockaddr_in src_addr;
    socklen_t src_addr_len = sizeof(src_addr);

    while (!should_exit) {
        ssize_t recv_len = recvfrom(fd, buffer, sizeof(buffer), 0, (struct sockaddr*) &src_addr, &src_addr_len);
        if (recv_len < 0) {
            fprintf(stderr, "[Receive Messages] Receive from UDP socket failed: %s\n", strerror(errno));
            break;
        } else {
            printf("\n[Receive Messages] UDP packet from %s\n", inet_ntoa(src_addr.sin_addr));
            handle_rip_message((TRipPkt *)buffer, recv_len, src_addr.sin_addr);
        }
    }

    close(fd);
    return NULL;

}