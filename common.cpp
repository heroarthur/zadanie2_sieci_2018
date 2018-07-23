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


template <typename key, typename q_type>
class limited_fifo_dict {
private:
    uint32_t max_size;
    uint32_t psize;
    std::map<key,q_type> dict;
    std::queue<key> keys_fifo;
public:
    limited_fifo_dict (uint32_t max_size_, uint32_t psize_): max_size(max_size_), psize(psize_) {}
    bool pack_available(key k) {return dict.find(k) != dict.end();}
    bool dict_empty() {return dict.empty();}
    q_type get_packg (key k) {return *dict.find(k);}
    q_type get_next () {return *dict.find(keys_fifo.front());}
    key max_available_key() {return keys_fifo.back();};
    void insert(key k, q_type v) {
        keys_fifo.push(k);
        dict.emplace(k, v);
        if(dict.size() > max_size)
            dict.erase(keys_fifo.front()); keys_fifo.pop();
    }
};



struct packgs {
    uint64_t first_byte;
    string bytes;
};



class Input_packs_queue {
private:
    uint32_t psize;
    uint64_t first_byte_num;
    uint64_t next_package_num;
    string buff;
    string s;
    limited_fifo_dict<string, packgs> packs_dict;

public:
    Input_packs_queue(uint32_t psize_, uint32_t queue_size):
            packs_dict(queue_size, psize_), psize(psize_),
            first_byte_num(0), next_package_num(0) {}
    void load_packs_from_input();
    bool next_pack_available(string pack_key);
    bool pack_available(string pack_key);
    void get_next_pack(packgs& p);
    void get_pack(packgs& p, string pack_key);
    void read_input(char* buff);
};

void Input_packs_queue::load_packs_from_input() {
    static char tmp[128000];
    static string unfinished_pack;
    static uint32_t next_pack;
    read_input(tmp);
    string s = unfinished_pack + string(tmp);
    for(next_pack=0; next_pack+psize <= s.length(); next_pack += psize) {
        packs_dict.insert(to_string(first_byte_num),
                          packgs{first_byte_num, s.substr(next_pack, psize)});
        first_byte_num += psize;
    }
    unfinished_pack = s.substr(next_pack, psize);
}

void Input_packs_queue::get_next_pack(packgs& p) {
    static uint64_t ret_pack;
    if(packs_dict.dict_empty()) {p = nullptr; return;}
    while(!packs_dict.pack_available(to_string(next_package_num))) {
        next_package_num += psize;
    }
    ret_pack = next_package_num;
    next_package_num += psize;
    p = packs_dict.get_packg(to_string(ret_pack));
}

void Input_packs_queue::get_pack(packgs& p, string pack_key) {
    p = packs_dict.get_packg(pack_key);
}

bool Input_packs_queue::next_pack_available(string pack_key) {
    if(packs_dict.dict_empty()) return false;
    uint64_t k;
    string max_key = packs_dict.max_available_key();
    std::istringstream iss(max_key);
    iss >> k;
    return next_package_num <= k;
};


bool Input_packs_queue::pack_available(string pack_key) {
    return packs_dict.pack_available(pack_key);
}

void Input_packs_queue::read_input(char* buff) {
    memset(buff, 0, 128000);
    read(0, buff, 128000);
}