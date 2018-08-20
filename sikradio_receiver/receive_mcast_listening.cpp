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
#include<set>

#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "../common/common.hpp"

#include "receive_mcast_listening.hpp"



using namespace std;




void* receive_transmitters_identyfication(void *threat_data) {
    static char buff[RECVFROM_BUFF_SIZE];

    recv_transmitter_data* config = (recv_transmitter_data*)threat_data;
    int recv_sockfd = config->recv_sockfd;
    auto cv = config->cv;
    availabile_transmitters* transmitters = config->transmitters;
    Connection_addres* broadcast_con = config->broadcast_con;
    string broadcast_message = config->broadcast_message;

    static struct timeval tv{0,0};
    tv.tv_sec = 1;
    tv.tv_usec = 500000;

    fd_set master;
    fd_set readfds;
    FD_ZERO(&master);
    FD_SET(recv_sockfd, &master);


    struct sockaddr their_addr;
    socklen_t addr_len = sizeof their_addr;



    recv_msg recv_identyfication;
    bool continue_recv_identyfication = true;
    while(continue_recv_identyfication) {
        readfds = master; // copy it
        if (select(recv_sockfd+1, &readfds, nullptr, nullptr, &tv) == -1) {
            perror("select");
            exit(1);
        }
        ssize_t numbytes;
        if (FD_ISSET(recv_sockfd, &readfds)) {
            if ((numbytes = recvfrom(recv_sockfd, buff, RECVFROM_BUFF_SIZE - 1,
                    0, &their_addr, &addr_len)) == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    continue;
                } else {
                    perror("recvfrom");
                    continue;
                }
            }
            recv_identyfication.text = string(buff);
            recv_identyfication.sender_addr.ai_addr = their_addr;
            recv_identyfication.sender_addr.ai_addrlen = addr_len;

            if (!msgIsBorewicz(recv_identyfication.text)) continue;
            transmitter_addr new_transmitter;
            parse_identyfication(recv_identyfication, new_transmitter);
            transmitters->update_transmitter(new_transmitter);
            cv->notify_all();
        } else {
            tv.tv_sec = 5;
            sendto_msg(recv_sockfd, *broadcast_con, ZERO_SEVEN_COME_IN.c_str(),
                       ZERO_SEVEN_COME_IN.length()+1);
        }
    }
    pthread_exit(nullptr);
}




void* write_packages_to_stdin(void *threat_data) {
    stdin_write_data* config = (stdin_write_data*)threat_data;
    list< packgs_set_to_stdin >* stdin_packs =  config->stdin_packs;
    std::condition_variable* cv_stdin_packgs = config->cv_stdin_packgs;
    pthread_mutex_t* stdin_list_mutex = config->stdin_list_mutex;
    current_transmitter_session* session = config->session;

    char buff[100000];

    std::mutex mtx;
    std::unique_lock<std::mutex> lck(mtx);

    packgs_set_to_stdin stdin_packgs{};



    bool loop = true;
    while(loop) {
        while(stdin_packs->empty()) cv_stdin_packgs->wait(lck);
        pthread_mutex_lock(stdin_list_mutex);
        if(!stdin_packs->empty()) {
            stdin_packgs.psize = stdin_packs->front().psize;
            stdin_packgs.first_byte_num = stdin_packs->front().first_byte_num;
            stdin_packgs.packgs.swap(stdin_packs->front().packgs);
            stdin_packs->pop_front();
            pthread_mutex_unlock(stdin_list_mutex);
        }
        else {
            pthread_mutex_unlock(stdin_list_mutex);
            continue;
        }

        auto it = stdin_packgs.packgs.find(stdin_packgs.first_byte_num);

        uint64_t last_key = stdin_packgs.first_byte_num - stdin_packgs.psize;
        uint64_t cur_key;
        for(;it != stdin_packgs.packgs.end(); it++) {
            cur_key = it->first;
            if(cur_key - last_key != stdin_packgs.psize) {
                session->SESSION_ESTABLISHED = false;
                continue;
            }

            it->second.write_to_array(buff, 0, stdin_packgs.psize);
            fwrite(buff, 1, stdin_packgs.psize, stdout);
            last_key = cur_key;
        }
    }
    pthread_exit(nullptr);
}

