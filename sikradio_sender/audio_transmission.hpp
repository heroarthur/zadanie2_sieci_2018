#ifndef ZADANIE2_AUDIO_TRANSMISSION_HPP
#define ZADANIE2_AUDIO_TRANSMISSION_HPP
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

#include "../common/common.hpp"
#include "../common/datagram_packing.hpp"

using namespace std;


bool next_retransmission_time(const uint32_t &rtime);

void create_audio_pack(uint64_t session_id, packgs &p, char *tr_pack);

void emit_series_of_ordered_packages(int sockfd, Connection_addres& con, Input_management &input_queue);

void emit_single_package(int sockfd, Connection_addres& con, pack_id id, Input_management &input_queue);

void packs_retransmission(int sockd, Connection_addres& con,
                          concurrent_uniqe_list<string> &ret_list,
                          Input_management &input_queue);


void write_sikradio_sender_arguments(string mcast_addr, string nazwa_nadajnika,
                                     string data_port, uint32_t ctrl_port,
                                     uint32_t psize, uint32_t fsize, uint32_t rtime);

void set_sikradio_sender_arguments(const int& argc, char **argv,
                                   string& mcast_addr, string& nazwa_nadajnika,
                                   string& data_port, string& ctrl_port,
                                   uint32_t& psize, uint32_t& fsize, uint32_t& rtime);

void assign_sikradio_sender_default_arguments(string& nazwa_nadajnika,
                                              string& data_port, string& ctrl_port,
                                              uint32_t& psize, uint32_t& fsize, uint32_t& rtime);



#endif //ZADANIE2_AUDIO_TRANSMISSION_HPP
