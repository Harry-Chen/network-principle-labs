#include "rt.h"
#include "common.h"

#include <functional>
#include <vector>
#include <cstdio>

struct Route {
    uint32_t ip;
    uint32_t prefix;
    uint32_t index;

    bool operator>(const Route &rhs) {
        return this->prefix > rhs.prefix;
    }

    bool operator==(const Route &rhs) {
        return this->ip == rhs.ip && this->prefix == rhs.prefix;
    }

    bool operator==(int ip) {
        return ((uint32_t)ip & PREFIX_DEC2BIN(this->prefix)) == this->ip;
    }
};

std::vector<Route> *table;

void rt_init() {
    table = new std::vector<Route>();
}

void rt_insert(uint32_t ip, uint32_t prefix, uint32_t index) {
    table->push_back({ip, prefix, index});
    in_addr addr;
    addr.s_addr = htonl(ip);
    printf("Insert: %s/%d index %d\n", inet_ntoa(addr), prefix, index);
    std::sort(table->begin(), table->end(), std::greater<>());
}

void rt_remove(uint32_t ip, uint32_t prefix) {
    Route route{ip, prefix, 0};
    table->erase(std::remove(table->begin(), table->end(), route), table->end());
    in_addr addr;
    addr.s_addr = htonl(ip);
    printf("Remove: %s/%d\n", inet_ntoa(addr), prefix);
    std::sort(table->begin(), table->end(), std::greater<>());
}

uint32_t rt_match(uint32_t ip, uint32_t prefix, uint32_t exact) {
    in_addr addr;
    addr.s_addr = htonl(ip);
    // printf("Match: %s/%d mode %s\n", inet_ntoa(addr), prefix, exact ? "exact" : "longest");
    if (exact) {
        Route route{ip, prefix, 0};
        auto iter = std::find(table->begin(), table->end(), route);
        return (iter != table->end()) ? iter->index : 0;
    } else {
        auto iter = std::find(table->begin(), table->end(), ip);
        return (iter != table->end()) ? iter->index : 0;
    }
}

uint32_t rt_lookup(uint32_t ip) {
    return rt_match(ip, 0, 0);
}

void rt_cleanup() {
    delete table;
    table = nullptr;
}

uint32_t rt_iterate(uint32_t nextOf) {

    auto iter = table->begin();

    if (nextOf != 0) {
        while (iter != table->end()) {
            if (iter->index == nextOf) {
                iter++;
                break; 
            }
            iter++;
        }
    }

    auto result = (iter != table->end()) ? iter->index : -1;
    // printf("Iterate: nextOf %d is %d\n", nextOf, result);
    
    return result;
}
