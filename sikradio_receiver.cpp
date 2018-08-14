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


#include <iostream>

#include <condition_variable>

#include <thread>

#include <chrono>

#include <array>

#include "common/common.hpp"

#include "sikradio_receiver/receive_managment.hpp"
#include "sikradio_receiver/receive_mcast_listening.hpp"


using namespace std;


template <uint32_t timeout_sec>
bool recv_before_timeout(current_transmitter_session &session, availabile_transmitters& transmitters) {
    static struct timeval tv{0,0};
    tv.tv_sec = timeout_sec;

    select(session.mcast_sockfd+1, &session.mcast_fd_set, nullptr, nullptr, &tv);

    if (FD_ISSET(session.mcast_sockfd, &session.mcast_fd_set))
        return true;
    transmitters.clear_not_reported_transmitters();
    return (session.SESSION_ESTABLISHED = false);
}

bool transmitter_availabile(availabile_transmitters& tr) {return !tr.empty();}


void add_missig_packgs_to_list(uint32_t psize, concurrent_uniqe_list<string>& missing_packs, limited_dict<uint64_t, byte_container>& recv_packs) {
    uint64_t previous_pack, highest_pack;
    static list<string> missings;
    if(!recv_packs.two_highest_keys(previous_pack, highest_pack))
        return;
    missings.clear();
    printf("diff %d \n", highest_pack - previous_pack - psize);
    for(uint64_t m = previous_pack + psize; m < previous_pack; m += psize) {
        missings.emplace_back(to_string(m));
    }
    missing_packs.insert(missings);
}

void transfer_packgs_to_stdin_thread(current_transmitter_session& session,
                                     list< packgs_set_to_stdin >& stdin_packs,
                                     pthread_mutex_t& stdin_list_mutex,
                                     std::condition_variable& cv_stdin_packgs) {
   stdin_write_data d;
   map<uint64_t, byte_container> packgs_map;
   session.packs_dict->ret_underlying_map(packgs_map);
   pthread_mutex_lock(&stdin_list_mutex);
   stdin_packs.emplace_back(packgs_set_to_stdin{});
   stdin_packs.back().psize = session.psize;
   stdin_packs.back().first_byte_num = session.byte0;
   stdin_packs.back().packgs.swap(packgs_map);
   pthread_mutex_unlock(&stdin_list_mutex);
   cv_stdin_packgs.notify_all();

   session.FIRST_PACKS_RECEIVED = false;
}


int main (int argc, char *argv[]) {
    string discover_addr;
    string ui_port;
    string ctrl_port;
    string data_port;
    uint32_t bsize;
    uint32_t rtime;


    set_default_receiver_arguments(discover_addr, ui_port, ctrl_port, data_port, bsize, rtime);

    //set_sikradio_receiver_arguments(argc, argv, discover_addr, ui_port, ctrl_port, bsize, rtime);
    transmitters_set finded_transmitters;

    uint16_t ctrl_port_num = parse_optarg_to_number(0, ctrl_port.c_str());


    int broadcast_sockfd;
    Connection_addres broadcast_location{};
    get_communication_addr(broadcast_location, discover_addr.c_str(), ctrl_port.c_str());
    broadcast_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    int broadcast = 1;
    setsockopt(broadcast_sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof broadcast);


    int send_rexmit_sockfd;
    send_rexmit_sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    int recv_senders_id;
    recv_senders_id = socket(AF_INET, SOCK_DGRAM, 0);
    Connection_addres recv_senders_con{};
    get_communication_addr(recv_senders_con, USE_MY_IP, ctrl_port.c_str());
    int l = 1;
    setsockopt(recv_senders_id, SOL_SOCKET, SO_REUSEADDR, &l, sizeof(int));
    bind_socket(recv_senders_id, recv_senders_con);
    //fcntl(recv_senders_id, F_SETFL, O_NONBLOCK);
    availabile_transmitters transmitters;




    struct sockaddr their_addr;
    socklen_t addr_len;
    fd_set readfds;



    current_transmitter_session session;
    limited_dict<uint64_t , byte_container> d(bsize);
    session.packs_dict = &d;
    concurrent_uniqe_list<string> rexmit_packgs_numbers;



    std::mutex mtx;
    std::condition_variable cv;
    std::unique_lock<std::mutex> lck(mtx);

    pthread_t recv_identyfication;
    struct recv_transmitter_data thread_identyfication_conf{recv_senders_id, &cv, &transmitters};
    if(pthread_create(&recv_identyfication, nullptr, receive_transmitters_identyfication, (void *)&thread_identyfication_conf)) {
        printf("Error:unable to create identyfication receive thread");
        exit(1);
    }


    pthread_t thread_broadcaster;
    struct send_broadcast_data thread_broadcast_conf{broadcast_sockfd, ZERO_SEVEN_COME_IN, broadcast_location};
    if(pthread_create(&thread_broadcaster, nullptr, send_broadcast, (void *)&thread_broadcast_conf)) {
        printf("Error:unable to create broadcast thread");
        exit(1);
    }




    list< packgs_set_to_stdin > stdin_packs;
    pthread_mutex_t	stdin_list_mutex;
    std::condition_variable cv_stdin;
    stdin_list_mutex = PTHREAD_MUTEX_INITIALIZER;
    if (pthread_mutex_init(&stdin_list_mutex, nullptr) != 0)
    {
        printf("\n stdin_list_mutex list mutex initialization failed\n");
        exit(1);
    }

    pthread_t thread_writer;
    struct stdin_write_data thread_writer_data{&stdin_packs,  &cv_stdin, &stdin_list_mutex};
    if(pthread_create(&thread_broadcaster, nullptr, write_packages_to_stdin, (void *)&thread_writer_data)) {
        printf("Error:unable to create broadcast thread");
        exit(1);
    }


    static ROUND_TIMER rexmit_timer(rtime);
    bool t = true;








    byte_container recv_raw_bytes;
    while(t) {
        if (transmitters.empty()) {cv.wait(lck);}

        if(!session.SESSION_ESTABLISHED) {
            restart_audio_player(session, transmitters, ctrl_port_num);
            continue;
        }
        if(!recv_before_timeout<20>(session, transmitters))// session.mcast_sockfd, session.mcast_fd_set);
            continue;

        clear_not_reported_transmitters(finded_transmitters);
        ssize_t numbytes;

        uint64_t session_id;
        uint64_t first_byte_num;
        static char buff[RECVFROM_BUFF_SIZE];



        //DEBUG
        static uint64_t BRAKUJACYCH_PAKIETOW = 0;
        static uint64_t ODEBRANYCH_PAKIETOW = 0;
        static uint64_t last_packg_num = 0;

        if(!session.SESSION_ESTABLISHED) continue;
        if ((numbytes = recvfrom(session.mcast_sockfd, buff, RECVFROM_BUFF_SIZE-1 , 0, nullptr, nullptr)) == -1) {
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
        //printf("%d \n", first_byte_num);

        //DEBUG
            if(first_byte_num - last_packg_num != PSIZE_DEF)
                BRAKUJACYCH_PAKIETOW++;
            last_packg_num = first_byte_num;
            ODEBRANYCH_PAKIETOW++;
            //DEBUG

         if(!session.FIRST_PACKS_RECEIVED) update_session_first_pack(session_id, first_byte_num, recv_psize, session);

         if(session_id < session.session_id) continue;
         if(session_id > session.session_id) {
            session.SESSION_ESTABLISHED = false;
            continue;
         }
        //session.packs_dict->

        recv_raw_bytes.create_new(buff, packg_meta_number_len, session.psize);
        session.packs_dict->insert(first_byte_num, recv_raw_bytes);
        if(session.packs_dict->length() > (3/4) * bsize) {
            transfer_packgs_to_stdin_thread(session, stdin_packs, stdin_list_mutex, cv_stdin);
            rexmit_packgs_numbers.clear();
        }
        add_missig_packgs_to_list(session.psize, rexmit_packgs_numbers, *session.packs_dict);
    }



}





