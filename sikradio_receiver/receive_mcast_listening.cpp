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


//#include "../sikradio_receiver/receive_managment.hpp"

using namespace std;


/*
void restart_audio_player(current_transmitter_session& session, transmitters_set& availabile_transmiters, uint16_t new_rexmit_port) {
    if(availabile_transmiters.empty()) return;
    transmitter_addr new_tr = *availabile_transmiters.begin();
    init_transmitter_session(session, new_tr, new_rexmit_port);
    create_socket_binded_to_new_mcast_addr(session.mcast_sockfd, session.mcast_addr.c_str(), session.data_port.c_str());
    session.SESSION_ESTABLISHED = true;
}
*/

/*
void manage_receive_audio(int rexmit_sockfd, uint16_t rexmit_port, const uint32_t bsize, transmitters_set& availabile_transmiters, const uint32_t& rexmit_time, current_transmitter_session& session) {
    static ROUND_TIMER rexmit_timer(rexmit_time);

    if(!session.SESSION_ESTABLISHED) {
        restart_audio_player(session, availabile_transmiters, rexmit_port);
    }
    if(!session.SESSION_ESTABLISHED) return;
    //receive_pending_packs(session);
    //update_rexmit(session);
    //pobierz ile sie da, ustawiajac session id i numer pierwszej paczki
    //if(rexmit_timer.new_round_start()) {//kolejny rexmit time
    //    send_rexmit(rexmit_sockfd, session);
    //}
    //write_audio_to_stdin(session);
}
*/


void* send_broadcast(void *threat_data) {
    usleep(1000); //wait for recv_identyfication thread to set up
    send_broadcast_data* config = (send_broadcast_data*)threat_data;

    int broadcast_sockfd = config->broadcast_sockfd;
    string broadcast_message = config->broadcast_message;
    Connection_addres broadcast_connetion = config->broadcast_location;

    bool continue_requests = true;
    while(continue_requests) {
        sendto_msg(broadcast_sockfd, broadcast_connetion, broadcast_message.c_str(), broadcast_message.length());
        sleep(5);
    }
}




void* receive_transmitters_identyfication(void *threat_data) {
    static char buff[RECVFROM_BUFF_SIZE];

    recv_transmitter_data* config = (recv_transmitter_data*)threat_data;
    int recv_sockfd = config->recv_sockfd;
    auto cv = config->cv;
    availabile_transmitters* transmitters = config->transmitters;

    struct sockaddr their_addr;
    socklen_t addr_len;
    recv_msg recv_identyfication;
    bool continue_recv_identyfication = true;
    while(continue_recv_identyfication) {
        if (recvfrom(recv_sockfd, buff, RECVFROM_BUFF_SIZE-1 , 0, &their_addr, &addr_len) == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            }
            else {
                perror("recvfrom");
                continue;
            }
        }
        recv_identyfication.text = string(buff);
        recv_identyfication.sender_addr.ai_addr = their_addr;
        recv_identyfication.sender_addr.ai_addrlen = addr_len;

        if(!msgIsBorewicz(recv_identyfication.text)) continue;
        printf("recv id: %s \n", recv_identyfication.text.c_str());
        transmitter_addr new_transmitter;
        parse_identyfication(recv_identyfication, new_transmitter);
        transmitters->update_transmitter(new_transmitter);
        cv->notify_all();
    }
}




void* write_packages_to_stdin(void *threat_data) {
    stdin_write_data* config = (stdin_write_data*)&threat_data;
    list< packgs_set_to_stdin >* stdin_packs =  config->stdin_packs;
    std::condition_variable* cv_stdin_packgs = config->cv_stdin_packgs;
    pthread_mutex_t* stdin_list_mutex = config->stdin_list_mutex;

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

        uint64_t last_key = stdin_packgs.first_byte_num;
        uint64_t cur_key;
        for(;it != stdin_packgs.packgs.end(); it++) {
            cur_key = it->first;
            if(cur_key - last_key != stdin_packgs.psize) {
                continue;
            }
            it->second.write_to_array(buff, 0, stdin_packgs.psize);
            if(fwrite(buff, 1, stdin_packgs.psize, stdout) != stdin_packgs.psize) {
                perror("fwrite");
            }
        }
    }

}



void* listening_mcast_packgs(void *thread_data) {
    mcast_listening_thread_configuration* config = (mcast_listening_thread_configuration*)thread_data;
    current_transmitter_session* session = (config->session);

    pthread_mutex_t	new_session_wait;
    new_session_wait = PTHREAD_MUTEX_INITIALIZER;
    if (pthread_mutex_init(&new_session_wait, nullptr) != 0)
        {
            printf("\n limited_concurrent_dict mutex initialization failed\n");
            exit(1);
        }

    ssize_t numbytes;

    uint64_t session_id;
    uint64_t first_byte_num;
    byte_container recv_raw_bytes{};
    static char buff[RECVFROM_BUFF_SIZE];



    //DEBUG
    static uint64_t BRAKUJACYCH_PAKIETOW = 0;
    static uint64_t ODEBRANYCH_PAKIETOW = 0;
    static uint64_t last_packg_num = 0;


    int loop = true;
    while(loop) {
        if ((numbytes = recvfrom(session->mcast_sockfd, buff, RECVFROM_BUFF_SIZE-1 , 0, nullptr, nullptr)) == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            }
            else {
                perror("recvfrom");
                continue;
            }
        }
        get_int64_bit_value(buff, session_id, 0);
        get_int64_bit_value(buff, first_byte_num, sizeof(uint64_t));
        uint32_t recv_psize = numbytes - 2*sizeof(uint64_t);
        uint64_t packg_meta_number_len = 2*sizeof(uint64_t);

        //DEBUG
        if(first_byte_num - last_packg_num != PSIZE_DEF)
            BRAKUJACYCH_PAKIETOW++;
        last_packg_num = first_byte_num;
        ODEBRANYCH_PAKIETOW++;
        //DEBUG

        if(!session->FIRST_PACKS_RECEIVED) update_session_first_pack(session_id, first_byte_num, recv_psize, *session);

        if(session_id < session->session_id) continue;
        if(session_id > session->session_id) {
            session->SESSION_ESTABLISHED = false;
            continue;
        }

        recv_raw_bytes.create_new(buff, packg_meta_number_len, session->psize);
        session->packs_dict->insert(first_byte_num, recv_raw_bytes);
        //session.biggest_received_pack = max<uint64_t>(session.biggest_received_pack, first_byte_num); TODO end()-1 simply should have biggest key
        //printf("received packed %d \n", first_byte_num - session.byte0); //TODO
        printf("BRAKUJACE_PAKIETY %d \n", BRAKUJACYCH_PAKIETOW); //TODO
    }
}