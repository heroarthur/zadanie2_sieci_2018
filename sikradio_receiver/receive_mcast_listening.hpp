//
// Created by karol on 12.08.18.
//

#ifndef ZADANIE2_RECEIVE_MCAST_LISTENING_HPP
#define ZADANIE2_RECEIVE_MCAST_LISTENING_HPP


#include "../common/common.hpp"
#include "receive_managment.hpp"

#include <iostream>
#include <condition_variable>
#include <thread>
#include <chrono>


struct mcast_listening_thread_configuration {
    current_transmitter_session* session;
};
void* listening_mcast_packgs(void *threat_data);



struct send_broadcast_data {
    int broadcast_sockfd;
    string broadcast_message;
    Connection_addres broadcast_location;
};
void* send_broadcast(void *threat_data);



struct recv_transmitter_data {
    int recv_sockfd;
    std::condition_variable* cv;
    availabile_transmitters* transmitters;
};
void* receive_transmitters_identyfication(void *threat_data);



struct packgs_set_to_stdin {
    uint32_t psize;
    uint64_t first_byte_num;
    std::map<uint64_t, byte_container> packgs;
};

struct stdin_write_data {
    list< packgs_set_to_stdin >* stdin_packs;
    std::condition_variable* cv_stdin_packgs;
    pthread_mutex_t* stdin_list_mutex;
    current_transmitter_session* session;
};
void* write_packages_to_stdin(void *threat_data);



#endif //ZADANIE2_RECEIVE_MCAST_LISTENING_HPP
