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
#include <set>
#include <map>


#include <sys/time.h>
#include <atomic>

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



const string ZERO_SEVEN_COME_IN = "ZERO_SEVEN_COME_IN";
const string LOUDER_PLEASE = "LOUDER_PLEASE";
const string BOREWICZ_HERE = "BOREWICZ_HERE";






bool msgIsLookup(string msg);
bool msgIsRexmit(string msg);
bool msgIsBorewicz(string msg);


const uint32_t second = 1;
const uint32_t third = 2;
const uint32_t from_fourth = 4;

const uint32_t RECVFROM_BUFF_SIZE = 10000;



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


uint64_t current_time_sec();



struct transmitter_addr{
    Connection_addres direct_rexmit_con;
    string mcast_addr;
    string data_port;
    string nazwa_stacji;
    mutable uint64_t last_reported_sec;
};

struct transmitter_comp {
    bool operator() (const transmitter_addr& lhs,
                     const transmitter_addr& rhs) const
    {
        if(lhs.nazwa_stacji == rhs.nazwa_stacji)
            return 0 < memcmp(&lhs.direct_rexmit_con.ai_addr,
                              &rhs.direct_rexmit_con.ai_addr,
                              sizeof(sockaddr));
        else
            return lhs.nazwa_stacji < rhs.nazwa_stacji;
    }
};



template<uint64_t diff>
bool last_report_older_than(uint64_t last_report_sec) {
    static struct timeval tval;
    if(gettimeofday(&tval, nullptr) == 0) {
        uint64_t current_time_sec = (uint64_t)tval.tv_sec;
        return current_time_sec > diff + last_report_sec;
    }
    return false;
}


bool validate_port(string& port);




typedef std::set<transmitter_addr, transmitter_comp> transmitters_set;

class availabile_transmitters {
public:
    transmitters_set transmitters;
    pthread_mutex_t	mutex_protection = PTHREAD_MUTEX_INITIALIZER;
    uint32_t choosen_transmitter = 0;
    std::atomic<bool> TRANSMITTER_NUMBER_CHANGED;


public:
    availabile_transmitters () {
        if (pthread_mutex_init(&mutex_protection, nullptr) != 0)
        {
            printf("\n availabile_transmitters mutex_protection initialization failed\n");
            exit(1);
        }
    }

    void update_transmitter(transmitter_addr& new_transmitter) {
        pthread_mutex_lock(&mutex_protection);
        transmitters_set::iterator tr = transmitters.find(new_transmitter);
        if(tr != transmitters.end()) {
            (tr)-> last_reported_sec = current_time_sec();
        }
        else {
            TRANSMITTER_NUMBER_CHANGED = true;
            transmitters.insert(new_transmitter);
        }
        pthread_mutex_unlock(&mutex_protection);
    }

    bool empty() {
        pthread_mutex_lock(&mutex_protection);
        bool empty = transmitters.empty();
        pthread_mutex_unlock(&mutex_protection);
        return empty;
    }

    bool get_next_transmitter(transmitter_addr& ret_transmitter,
                              transmitter_addr& current_transmitter) { // TODO  szukanie nazwy transmitera, szukanie poprzedniego transmitera
        pthread_mutex_lock(&mutex_protection);
        if(!transmitters.empty()) {
            auto it = transmitters.find(current_transmitter);
            if (transmitters.end() != it) {
                    ret_transmitter = *it;
            }
            else {
                ret_transmitter = *transmitters.begin();
            }
            pthread_mutex_unlock(&mutex_protection);
            return true;
        }
        pthread_mutex_unlock(&mutex_protection);
        return false;
    }

    bool get_choosen_tansmitter(transmitter_addr& ret_transmitter) {
        pthread_mutex_lock(&mutex_protection);
        if(transmitters.empty()) {
            pthread_mutex_unlock(&mutex_protection);
            return false;
        }
        if(transmitters.size() < choosen_transmitter+1) {
            printf("give choosen transmitter error");
            exit(1);
        }
        auto it = transmitters.begin();
        std::advance(it, choosen_transmitter);
        ret_transmitter = *it;
        pthread_mutex_unlock(&mutex_protection);
        return true;
    }


    void clear_not_reported_transmitters() {
        pthread_mutex_lock(&mutex_protection);
        for (const transmitter_addr& tr : transmitters) {
            if(last_report_older_than<20>(tr.last_reported_sec)) {
                transmitters.erase(tr);
                choosen_transmitter = 0;
                TRANSMITTER_NUMBER_CHANGED = true;
            }
        }
        pthread_mutex_unlock(&mutex_protection);
    }


    void give_transmitters_names(list<string>& transmitter_names,
                                 uint32_t& row, uint32_t& line) {
        transmitter_names.clear();

        pthread_mutex_lock(&mutex_protection);
        if(transmitters.empty()){
            pthread_mutex_unlock(&mutex_protection);
            return;
        }
        row = this->choosen_transmitter;

        line = 3;
        row += 4; //---SIK RADIO ---
        for (auto tr = transmitters.begin(); tr != transmitters.end(); tr++) {
            transmitter_names.emplace_back("     " + tr->nazwa_stacji);
        }
        std::list<string>::iterator it = transmitter_names.begin();
        std::advance(it, this->choosen_transmitter);
        if(transmitter_names.empty()) {printf("transmitter_names empty!!!");}
        (*it)[2] = '>';
        pthread_mutex_unlock(&mutex_protection);
    }


    void change_transmitter(int move) {
        pthread_mutex_lock(&mutex_protection);
        int new_transmitter_index = choosen_transmitter + move;
        this->choosen_transmitter = (uint32_t)max(0, new_transmitter_index);
        this->choosen_transmitter = (uint32_t)min(max((int)transmitters.size()-1,0), (int)choosen_transmitter);
        pthread_mutex_unlock(&mutex_protection);
    }


};





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
    for(auto it = v.begin(); it != (v.end()--); it++)
        s += *it + delimiter;
    if(!s.empty())
        s.pop_back();
    return s;
}



uint32_t parse_optarg_to_number(int option, const char *optarg);



template <typename key, typename q_type>
class limited_dict {
private:
    uint32_t max_size;
    std::map<key,q_type> dict;
    std::queue<key> keys_fifo;

public:
    limited_dict (uint32_t max_size_): max_size(max_size_) {}
    bool two_highest_keys(key& previous, key& highest) {
        if(dict.size() < 2) return false;
        previous = prev(dict.end(), 2)->first;
        highest = prev(dict.end(), 1)->first;
        return true;
    };
    void ret_underlying_map(map<key,q_type>& ret_map) {
        ret_map.swap(dict); dict.clear();
        while(keys_fifo.size()>0) keys_fifo.pop();}

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
        if(dict.size() > max_size) {
            dict.erase(keys_fifo.front()); keys_fifo.pop();
        }
    }
};





template <typename key, typename q_type>
class limited_concurrent_map {
private:
    uint32_t max_size;
    std::map<key,q_type> dict;
    pthread_mutex_t	mutex;

public:
    limited_concurrent_map (uint32_t max_size_): max_size(max_size_) {
        mutex = PTHREAD_MUTEX_INITIALIZER;
        if (pthread_mutex_init(&mutex, nullptr) != 0)
        {
            printf("\n limited_concurrent_dict mutex_protection initialization failed\n");
            exit(1);
        }
    }
    void set_max_size(uint32_t new_max_size) {max_size = new_max_size;}

    void clear() {
        pthread_mutex_lock(&mutex);
        dict.clear();
        pthread_mutex_unlock(&mutex);
    }

    void insert(key elem_id, q_type elem) {
        pthread_mutex_lock(&mutex);
        if(dict.find(elem_id) != dict.end()) {
            dict[elem_id] = elem;
        }
        pthread_mutex_unlock(&mutex);
    }
    void ret_uniqe_map(std::map<key,q_type> ret_map) {
        ret_map.clear();
        pthread_mutex_lock(&mutex);
        ret_map.merge(dict);
        dict.clear();
        pthread_mutex_unlock(&mutex);
    }
};







template <typename q_type>
class concurrent_uniqe_list {
public:
    std::list<q_type> l;
    pthread_mutex_t	mutex;


public:
    concurrent_uniqe_list() {
        mutex = PTHREAD_MUTEX_INITIALIZER;
        if (pthread_mutex_init(&mutex, nullptr) != 0)
        {
            printf("\n mutex_protection init failed\n");
            exit(1);
        }
    }
    void insert(list<q_type>& insert_l) {
        pthread_mutex_lock(&mutex);
        l.splice(l.end(), insert_l);
        pthread_mutex_unlock(&mutex);
    }
    void ret_uniqe_list(list<q_type>& ret_l) {
        ret_l.clear();
        pthread_mutex_lock(&mutex);
        l.unique();
        l.sort();
        ret_l.splice(ret_l.end(), l);
        pthread_mutex_unlock(&mutex);
    }
    void clear() {
        pthread_mutex_lock(&mutex);
        l.clear();
        pthread_mutex_unlock(&mutex);
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
    byte_container(vector<char> input_bytes, vector<char>::iterator end, uint64_t first_byte_num_) {
        bytes.clear(); bytes.swap(input_bytes); first_byte_num = first_byte_num_;
    };


    void clear() {bytes.clear();}
    void write_to_array(char* arr, uint32_t beg, ssize_t write_len) {
        for(uint32_t i = 0; i < bytes.size(); i++) {
            arr[beg + i] = bytes[i];
        }
    }
    void pop_to_container(size_t pop_len, uint64_t fb_num,
                          byte_container& new_container) {
        assert(bytes.size() >= pop_len);
        new_container = byte_container(bytes.begin(),
                bytes.begin()+pop_len, fb_num);
        bytes.erase(bytes.begin(), bytes.begin()+pop_len);
        first_byte_num += pop_len;
    }
    size_t size() {return bytes.size();}


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





typedef string pack_id;
class Input_management {
private:
    uint64_t first_byte_num;
    uint64_t next_unused_package_num;
    uint64_t next_availabile_package_num;

    uint32_t fsize;
    limited_dict<pack_id, byte_container> dict;

    void read_input(byte_container& msg);

public:
    uint32_t psize;
    uint64_t session_id;
    uint32_t audio_pack_size;
    Input_management(uint32_t psize_, uint32_t fsize_):
            first_byte_num(0),
            next_unused_package_num(0),
            next_availabile_package_num(0),
            fsize(fsize_),
            dict(fsize_/psize_),
            psize(psize_) {
        audio_pack_size = sizeof(session_id) + sizeof(first_byte_num) + psize;
        session_id = current_time_sec();
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
    explicit ROUND_TIMER (uint64_t interval_in_milisec_): interval_in_usecond(interval_in_milisec_*1000) {};

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
};









void get_int64_bit_value(const char* datagram, uint64_t& val, int beg);





void fill_connection_struct(Connection_addres &connection, struct addrinfo *servinfo);
void get_communication_addr(Connection_addres& connection, const char* ip_addr, const char* port);
void receive_pending_messages(int& sockfd, list<recv_msg>& messages);
void create_socket_binded_to_new_mcast_addr(int& mcast_sockfd, const char* mcast_addr, const char* data_port);
void sendto_msg(int& sockfd ,const Connection_addres& connection, const char* msg, const uint64_t msg_len);
void sendto_msg(int& sockfd ,Connection_addres connection, const char* msg, const uint64_t msg_len, uint16_t destination_port);


void bind_socket(int& sockfd, Connection_addres& connection);
void bind_to_first_free_port(int& sockfd);


inline uint64_t unrolled(std::string const& value);



#endif //ZADANIE2_COMMON_HPP
