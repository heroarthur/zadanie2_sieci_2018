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


const uint32_t numer_albumu = 371869;
const string DISCOVER_ADDR_DEF = "255.255.255.255";
const uint32_t DATA_PORT_DEF = 20000 + (numer_albumu % 10000);
const uint32_t CTRL_PORT_DEF = 30000 + (numer_albumu % 10000);
const uint32_t UI_PORT_DEF = 10000 + (numer_albumu % 10000);
const uint32_t PSIZE_DEF = 512;
const uint32_t BSIZE_DEF = 65536;
const uint32_t FSIZE_DEF = 131072;
const uint32_t RTIME_DEF = 250;
const string NAZWA_DEF = "Nienazwany Nadajnik";


uint32_t parse_optarg_to_number(int option, char *optarg);//zmine nazwe

struct packgs {
    uint64_t first_byte_num;
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
    uint32_t fsize;
    uint32_t INPUT_READ_SIZE = FSIZE_DEF;
    string buff;
    string s;
    limited_dict<pack_id, packgs> dict;

    void read_input(char* buff);

public:
    uint32_t psize;
    uint64_t session_id;
    uint32_t audio_pack_size;

    Input_management(uint32_t psize_, uint32_t fsize_):
            dict(fsize_, psize_), psize(psize_),
            first_byte_num(0), next_package_num(0), fsize(fsize_) {
        audio_pack_size = sizeof(session_id) + sizeof(first_byte_num) + psize;
    }
    bool next_pack_available();
    bool pack_available(pack_id id);
    void load_packs_from_input();
    void get_next_pack(packgs& p);
    void get_pack(packgs& p, pack_id id);
};



template <typename q_type>
class concurrent_uniqe_list {
private:
    uint32_t max_size;
    uint32_t psize;
    std::list<q_type> accepted_part;
    std::list<q_type> waiting_tail;
    pthread_mutex_t	mutex = PTHREAD_MUTEX_INITIALIZER;

    std::list<q_type> merge_accepted_with_tail() {
        pthread_mutex_lock(&mutex);
        accepted_part.splice(accepted_part.end(), waiting_tail);
        pthread_mutex_unlock(&mutex);
    }
public:
    concurrent_uniqe_list() {}
    std::list<q_type> insert(list<q_type> l) {
        pthread_mutex_lock(&mutex);
        waiting_tail.splice(waiting_tail.end(), l);
        pthread_mutex_unlock(&mutex);
    }
    void ret_uniqe_list(list<q_type>& l) {
        merge_accepted_with_tail();
        l.clear();
        accepted_part.unique();
        accepted_part.sort();
        l.splice(l.end(), accepted_part);
    }
};




#endif //ZADANIE2_COMMON_HPP