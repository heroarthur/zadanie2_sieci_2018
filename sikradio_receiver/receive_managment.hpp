#ifndef ZADANIE2_RECEIVE_MANAGMENT_HPP
#define ZADANIE2_RECEIVE_MANAGMENT_HPP


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
#include <set>
#include <atomic>


#include "../common/common.hpp"


using namespace std;



void set_default_receiver_arguments(string& discover_addr,
                                    string& ui_port,
                                    string& ctrl_port,
                                    string&data_port,
                                    uint32_t& bsize,
                                    uint32_t& rtime);





void set_sikradio_receiver_arguments(const int& argc, char **argv,
                                     string& discover_addr,
                                     string& ui_port,
                                     string& ctrl_port,
                                     uint32_t& bsize,
                                     uint32_t& rtime,
                                     string& nazwa_preferowanego_odbiornika);


struct current_transmitter_session {
    std::atomic<bool> SESSION_ESTABLISHED;
    bool FIRST_PACKS_RECEIVED;
    pthread_mutex_t	mutex;

    availabile_transmitters reported_transmitters;

    int mcast_sockfd;
    fd_set mcast_fd_set;
    string mcast_addr;
    string data_port;

    uint64_t last_pack_num;
    uint64_t cur_pack_num;

    transmitter_addr current_transmitter;
    limited_dict<uint64_t, byte_container>* packs_dict;

    uint64_t byte0;
    uint64_t session_id;
    uint32_t psize;
};


void update_session_first_pack(uint64_t recv_session_id,
                               uint64_t first_byte_num,
                               uint32_t recv_psize,
                               current_transmitter_session& session);

void init_transmitter_session(current_transmitter_session& session,
                              const transmitter_addr& tr);

void restart_audio_player(current_transmitter_session& session,
                          uint16_t ctrl_port_u16);

void send_rexmit(int rexmit_sockfd,
                 concurrent_uniqe_list<string>& missing_packs,
                 current_transmitter_session& session);


void parse_identyfication(const recv_msg& identyfication,
                          transmitter_addr& transmitter);


void clear_not_reported_transmitters(transmitters_set& transmitters);



#endif //ZADANIE2_RECEIVE_MANAGMENT_HPP
