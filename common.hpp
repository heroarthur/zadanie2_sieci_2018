#ifndef ZADANIE2_COMMON_HPP
#define ZADANIE2_COMMON_HPP

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

using namespace std;

uint32_t parse_optarg_to_number(int option, char *optarg);

class Input_fifo_queue {
private:
    uint32_t max_size;
    char tmp[128000];
    string buff;
    string s;

public:
    Input_fifo_queue() {}
    void read_input();
    string pop_part(uint32_t pop_size);
};



struct packgs {
    uint64_t first_byte;
    string bytes;
};


struct datagram_connection {
    int sockfd;
    struct addrinfo *p;
};



template <typename key, typename q_type>
class limited_dict {
private:
    uint32_t max_size;
    uint32_t psize;
    std::map<key,q_type> dict;
    std::queue<key> keys_fifo;
public:
    limited_dict (uint32_t max_size_, uint32_t psize_): max_size(max_size_), psize(psize_) {}
    bool contain(key k) {return dict.find(k) != dict.end();}
    bool empty() {return dict.empty();}
    q_type get(key k) {return *dict.find(k);}
    key last_inserted_key() {return keys_fifo.back();};
    void insert(key k, q_type v) {
        keys_fifo.push(k);
        dict.emplace(k, v);
        if(dict.size() > max_size) {
            dict.erase(keys_fifo.front()); keys_fifo.pop();
        }
    }
};


typedef string pack_id;
class Input_management {
private:
    uint64_t first_byte_num;
    uint64_t next_package_num;
    string buff;
    string s;
    limited_dict<pack_id, packgs> dict;

    void read_input(char* buff);

public:
    uint32_t psize;
    uint64_t session_id;
    uint32_t audio_pack_size;

    Input_management(uint32_t psize_, uint32_t fsize):
            dict(fsize, psize_), psize(psize_),
            first_byte_num(0), next_package_num(0) {
        audio_pack_size = sizeof(session_id) + sizeof(first_byte_num) + psize;
    }
    bool next_pack_available();
    bool pack_available(pack_id id);
    void load_packs_from_input();
    void get_next_pack(packgs& p);
    void get_pack(packgs& p, pack_id id);
};








#endif //ZADANIE2_COMMON_HPP
