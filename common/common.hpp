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
#include <queue>
#include <cassert>

#include <sys/time.h>

using namespace std;

#define USE_MY_IP nullptr


const uint32_t numer_albumu = 371869;
const string DISCOVER_ADDR_DEF = "255.255.255.255";
const uint32_t DATA_PORT_DEF = 20000 + (numer_albumu % 10000);
const uint32_t CTRL_PORT_DEF = 30000 + (numer_albumu % 10000);
const uint32_t UI_PORT_DEF = 10000 + (numer_albumu % 10000);
const uint32_t PSIZE_DEF = 512;
const uint32_t BSIZE_DEF = 65536;
const uint32_t FSIZE_DEF = 131072;
const uint32_t RTIME_DEF = 250;
const string NAZWA_DEF = "Nienazwany Nadajnik\0";



const string ZERO_SEVEN_COME_IN = "ZERO_SEVEN_COME_IN\0";
const string LOUDER_PLEASE = "LOUDER_PLEASE";
const string BOREWICZ_HERE = "BOREWICZ_HERE";

//string* CHOOSE_MY_IP = nullptr;



bool msgIsLookup(string msg);
bool msgIsRexmit(string msg);
bool msgIsBorewicz(string msg);


const uint32_t first = 0, from_first = 0;
const uint32_t second = 1, from_second = 1;
const uint32_t third = 2, from_third = 2;
const uint32_t fourth = 3, from_fourth = 3;


const uint32_t RECVFROM_BUFF_SIZE = 10000;
const uint32_t WRITE_BUFF_SIZE = 100000;


template<typename T>
T split_string_to_container(string s, string delimiter) {
    uint64_t start = 0U;
    uint64_t end = s.find(delimiter);
    T l;
    while (end != string::npos)
    {
        l.emplace_back(s.substr(start, end - start));
        start = end + delimiter.length();
        end = s.find(delimiter, start);
    }
    l.emplace_back(s.substr(start));
    return l;
}



template<typename T, uint32_t join_start>
string join_container_elements(T v, string delimiter) {
    string s = "";
    for(auto it = v.begin() + join_start; it != v.end(); it++)
        s += *it + delimiter;
    return s;
}


template<typename T>
string join_container_elements(T v, string delimiter) {
    string s = "";
    for(auto it = v.begin(); it != v.end(); it++)
        s += *it + delimiter;
    return s;
}




struct Recvfrom_msg {
    ssize_t msg_len;
    char* buff;
};


uint32_t parse_optarg_to_number(int option, const char *optarg);//zmien nazwe





template <typename key, typename q_type>
class limited_dict {
private:
    uint32_t max_size;
    std::map<key,q_type> dict;
    std::queue<key> keys_fifo;

public:
    limited_dict (uint32_t max_size_): max_size(max_size_) {}
    void set_max_size(uint32_t new_max_size) {max_size = new_max_size;}
    bool contain(key k) {return dict.find(k) != dict.end();}
    bool empty() {return dict.empty();}
    ssize_t length() {return dict.size();}
    q_type get(key k) {return (*dict.find(k)).second;}
    key last_inserted_key() {return keys_fifo.back();};
    void clear() {dict.clear(); while(keys_fifo.size()>0) keys_fifo.pop();}
    void insert(key k, q_type v) {
        keys_fifo.push(k);
        dict.emplace(k, v);
        uint64_t s = dict.size();
        if(dict.size() > max_size) {
            dict.erase(keys_fifo.front()); keys_fifo.pop();
        }
    }
};



template <typename q_type>
class concurrent_uniqe_list {
public:
    //uint32_t max_size;
    uint32_t psize;
    std::list<q_type> accepted_part;
    std::list<q_type> waiting_tail;
    pthread_mutex_t	mutex;

    void merge_accepted_with_tail() {
        pthread_mutex_lock(&mutex);
        accepted_part.splice(accepted_part.end(), waiting_tail);
        pthread_mutex_unlock(&mutex);

    }
public:
    concurrent_uniqe_list() {
        //pthread_mutex_lock(&mutex);
        mutex = PTHREAD_MUTEX_INITIALIZER;
        if (pthread_mutex_init(&mutex, nullptr) != 0)
        {
            printf("\n mutex init failed\n");
            exit(1);
        }
    }
    void insert(list<q_type>& l) { //std::list<q_type> insert(list<q_type>& l)
        pthread_mutex_lock(&mutex);
        waiting_tail.splice(waiting_tail.end(), l);
        pthread_mutex_unlock(&mutex);
    }
    void ret_uniqe_list(list<q_type>& l) {//    void ret_uniqe_list(list<q_type>& l) {

        merge_accepted_with_tail();

        l.clear();
        accepted_part.unique();
        accepted_part.sort();
        l.splice(l.end(), accepted_part);
    }
};



class byte_container {
private:
    vector<char> bytes;
public:
    uint64_t first_byte_num;

    byte_container() {bytes.clear(); first_byte_num = 0;}
    byte_container(vector<char>::iterator beg, vector<char>::iterator end, uint64_t first_byte_num_) {
        bytes.clear(); bytes.assign(beg, end); first_byte_num = first_byte_num_;
    };
    void clear() {bytes.clear();}
    void write_to_array(char* arr, uint32_t beg, ssize_t write_len) {
        for(int i = 0; i < bytes.size(); i++) {
            arr[beg + i] = bytes[i];
        }
    }
    byte_container pop_to_container(ssize_t pop_len, uint64_t fb_num) {
        assert(bytes.size() >= pop_len);
        byte_container new_container = byte_container(bytes.begin(), bytes.begin()+pop_len, fb_num);
        bytes.erase(bytes.begin(), bytes.begin()+pop_len);
        first_byte_num += pop_len;
        return new_container;
    }
    ssize_t size() {return bytes.size();}


    void create_new(const char* arr, uint32_t beg, ssize_t size) {
        bytes.clear();
        bytes.resize(size);
        for(uint32_t i = 0; i < size; i++) {
            bytes[i] = arr[i+beg];
        }
    }
    void emplace_back(const char* arr, uint32_t beg, ssize_t size) {
        for(uint32_t i = 0; i < size; i++) {
            bytes.emplace_back(arr[i+beg]);
        }
    }
};



struct packgs {
    uint64_t first_byte_num;
    //string bytes;
    byte_container bytes;
};

typedef string pack_id;
class Input_management {
private:
    FILE * stdin_debug_fd;

    uint64_t first_byte_num;
    uint64_t next_unused_package_num;
    uint64_t next_availabile_package_num;

    uint32_t fsize;
    uint32_t INPUT_READ_SIZE = FSIZE_DEF;
    string buff;
    string s;
    limited_dict<pack_id, byte_container> dict;

    void read_input(byte_container& msg);

public:
    uint32_t psize;
    uint64_t session_id = 123456789;
    uint32_t audio_pack_size;

    Input_management(uint32_t psize_, uint32_t fsize_):
            dict(fsize_/psize_), psize(psize_),
            first_byte_num(0), next_unused_package_num(0), fsize(fsize_), next_availabile_package_num(0) {
        //stdin_debug_fd=fopen("bytes_input", "r");
        //psize = (uint32_t)strlen("spitfire_package");
        audio_pack_size = sizeof(session_id) + sizeof(first_byte_num) + psize;

    }
    bool next_pack_available();
    bool pack_available(pack_id id);
    void load_packs_from_input();
    void get_next_pack(byte_container& p);
    void get_pack(byte_container& p, pack_id id);
};




const uint64_t MICROSECOND = 1000000;



class ROUND_TIMER {

private:
    uint64_t time_milisec_since_last_update;
    struct timeval tval;
    uint64_t interval_in_usecond;
    uint64_t tmp;

public:
    bool new_round_start() {
        if(gettimeofday(&tval, nullptr) == 0) {
            tmp = (uint64_t)tval.tv_usec + (uint64_t)tval.tv_sec*MICROSECOND;
            if(tmp >= interval_in_usecond + time_milisec_since_last_update) {
                time_milisec_since_last_update = (uint64_t)tval.tv_usec + (uint64_t)tval.tv_sec*MICROSECOND;
                return true;
            }
        }
        return false;
    }
    explicit ROUND_TIMER (uint64_t interval_in_milisec_) {interval_in_usecond = interval_in_milisec_*1000;}
};



uint64_t current_time_sec();


template<uint64_t diff>
bool last_report_older_than(uint64_t last_report_sec) {
    static struct timeval tval;
    if(gettimeofday(&tval, nullptr) == 0) {
        uint64_t current_time_sec = (uint64_t)tval.tv_sec;
        return current_time_sec > diff + last_report_sec;
    }
    return false;
}





//new connection design
struct Connection_addres {
    struct sockaddr ai_addr;
    socklen_t ai_addrlen;
    int ai_family;
    int ai_socktype;
    int ai_protocol;
};

struct recv_msg {
    string text;
    Connection_addres sender_addr;
};



void fill_connection_struct(Connection_addres &connection, struct addrinfo *servinfo);
void get_communication_addr(Connection_addres& connection, const char* ip_addr, const char* port);
void receive_pending_messages(int& sockfd, list<recv_msg>& messages);
void create_socket_binded_to_new_mcast_addr(int& mcast_sockfd, const char* mcast_addr, const char* data_port);
void sendto_msg(int& sockfd ,const Connection_addres& connection, const char* msg, const uint64_t msg_len);
void sendto_msg(int& sockfd ,Connection_addres connection, const char* msg, const uint64_t msg_len, uint16_t destination_port);
void bind_socket(int& sockfd, Connection_addres& connection);




#endif //ZADANIE2_COMMON_HPP
