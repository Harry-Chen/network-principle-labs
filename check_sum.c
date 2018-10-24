#include "check_sum.h"

uint16_t calculate_check_sum(struct ip* ip_header) {

    uint16_t original_checksum = ip_header->ip_sum;
    ip_header->ip_sum = 0;

    uint16_t *buf = (uint16_t *) ip_header;
    uint32_t sum = 0;

    int counter = (ip_header->ip_v & 0xF) * 4;

    while (counter > 1) {
        sum += *buf++;
        counter -= 2;
    }

    if (counter == 1) {
        sum += * (uint8_t *) buf;
    }

    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    ip_header->ip_sum = original_checksum;

    return ~sum;
}
