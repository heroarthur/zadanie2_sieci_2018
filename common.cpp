//
// Created by karol on 21.07.18.
//
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <regex>
#include <iostream>
#include <utility>
#include <string>
#include <arpa/inet.h>
#include <sstream>
#include <list>
#include <fcntl.h>
#include<ctime>
#include<climits>
#include <queue>

#include "common.hpp"

using namespace std;

uint32_t parse_optarg_to_number(int option, char *optarg) {
    char *end;
    constexpr int decimal = 10;
    uint64_t val = std::strtoul(optarg, &end, decimal);
    if ((errno == ERANGE && (val <= ULONG_MAX) || (errno != 0 && val == 0))) {
        printf("passed argument outside of range %c %s\n", (char)option, optarg);
        exit(1);
    }
    if (end == optarg) {
        printf("no digits were found %c %s\n", (char)option, optarg);
        exit(1);
    }
    if (*end == '\0' || end == optarg) {
        printf("argument have none-digit characters %c %s\n", (char)option, optarg);
        exit(1);
    }
    return (uint32_t)val;
}


void Input_management::load_packs_from_input() {
    static char tmp[128000];
    static string unfinished_pack;
    static uint32_t pack_start;
    read_input(tmp);
    string s = unfinished_pack + string(tmp);
    for(pack_start=0; pack_start+psize <= s.length(); pack_start += psize) {
        dict.insert(to_string(next_package_num),
                          packgs{next_package_num, s.substr(pack_start, psize)});
        next_package_num += psize;
    }
    unfinished_pack = s.substr(pack_start, psize);
}

void Input_management::get_next_pack(packgs& p) {
    while(!dict.contain(to_string(next_package_num))) {
        next_package_num += psize;
    }
    p = dict.get(to_string(next_package_num));
    next_package_num += psize;
}

void Input_management::get_pack(packgs& p, pack_id id) {
    p = dict.get(id);
}

bool Input_management::next_pack_available() {
    if(dict.empty()) return false;
    string max_key = dict.last_inserted_key();
    std::istringstream iss(max_key);
    uint64_t k; iss >> k;
    return next_package_num <= k;
};


bool Input_management::pack_available(pack_id id) {
    return dict.contain(id);
}

void Input_management::read_input(char* buff) {
    memset(buff, 0, 128000);
    read(0, buff, 128000);
}