#include "rip_message.h"
#include "routing_table.h"
#include "local_route.h"

static TRipPkt packet;

static int establish_rip_fd(in_addr_t dest) {
    int fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (fd < 0) {
        fprintf(stderr, "Error opening socket!\n");
        return -1;
    }
    
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0) {
        fprintf(stderr, "Set SO_REUSEADDR on socket failed!\n");
        return -1;
    }

    struct sockaddr_in local, remote;
    local.sin_family = AF_INET;
    local.sin_port = htons(RIP_PORT);
    local.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(fd, (struct sockaddr *)&local, sizeof(local)) < 0) {
        fprintf(stderr, "Bind to local RIP port failed!\n");
        return -1;
    }

    remote.sin_family = AF_INET;
    remote.sin_port = htons(RIP_PORT);
    remote.sin_addr.s_addr = dest;
    if (connect(fd, (struct sockaddr *)&remote, sizeof(remote)) < 0) {
        fprintf(stderr, "Connect to remote RIP port failed!\n");
        return -1;
    }

    return fd;
}

static void send_multicast_packet(int fd, void *data, uint32_t length, char* source) {
    for (int i = 0; i < MAX_IF; ++i) {
        if_info_t *iface = get_interface_info(i);
        if (iface->name[0] == '\0' || !iface->multicast) continue; // empty interface or cannot multicast
        if (setsockopt(fd, IPPROTO_IP, IP_MULTICAST_IF, &iface->ip, sizeof(struct in_addr)) < 0) {
            fprintf(stderr, "Set IP_MULTICAST_IF failed for %s with IP %s!\n", iface->name, inet_ntoa(iface->ip));
            continue;
        }
        fprintf(stderr, "[%s] Send from interface %s with IP %s, size %d...", source, iface->name, inet_ntoa(iface->ip), length);
        if (send(fd, data, length, 0) < 0) {
            fprintf(stderr, "Failed!\n");
        } else {
            fprintf(stderr, "Succeeded!\n");
        }
    }
}

void send_all_routes(in_addr_t dest) {

    int fd = establish_rip_fd(dest);
    if (fd < 0) return;

    bzero(&packet, sizeof(TRipPkt));

    packet.ucCommand = RIP_RESPONSE;
    packet.ucVersion = RIP_VERSION;
    int payload_size = fill_rip_packet(packet.RipEntries);
    
    int length = RIP_HEADER_LEN + sizeof(TRipEntry) * payload_size;

    printf("Start sending RIP update...\n");
    send_multicast_packet(fd, &packet, length, "Update");

    close(fd);


}

void send_rip_request() {

    int fd = establish_rip_fd(inet_addr(RIP_GROUP));
    if (fd < 0) return;

    if (setsockopt(fd, IPPROTO_IP, IP_MULTICAST_LOOP, &(int){ 1 }, sizeof(int)) < 0) {
        fprintf(stderr, "Set MULTICAST_LOOP failed!\n");
        return;
    }

    packet.ucCommand = RIP_REQUEST;
    packet.ucVersion = RIP_VERSION;
    
    printf("Start sending RIP request...\n");
    send_multicast_packet(fd, &packet, RIP_HEADER_LEN, "Request");

    close(fd);

}