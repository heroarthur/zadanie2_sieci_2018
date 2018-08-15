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

#include <pthread.h>

#include "common/common.hpp"
//#include "common/datagram_packing.hpp"

#include "sikradio_sender/audio_transmission.hpp"
#include "sikradio_sender/sikradio_listening.hpp"

using namespace std;




const ssize_t listener_buff_size = 1000;
//ctrl_port nasluchuje na rexmit i ZERO_SEVEN_COME_IN
//data_port na tym porcie receiver ma nasluchiwac na pakiety audio






int main (int argc, char *argv[]) {
    std::string mcast_addr = "224.0.0.3";
    std::string nazwa_nadajnika;
    std::string data_port;
    std::string ctrl_port;
    uint32_t psize;
    uint32_t fsize;
    uint32_t rtime;
    concurrent_uniqe_list<string> retransmision_requests;




    assign_sikradio_sender_default_arguments(nazwa_nadajnika, data_port,
                                             ctrl_port, psize, fsize, rtime);

    set_sikradio_sender_arguments(argc, argv, mcast_addr, nazwa_nadajnika,
                                  data_port, ctrl_port, psize, fsize, rtime);


    int mcast_socket;
    Connection_addres mcast_con{};
    get_communication_addr(mcast_con, mcast_addr.c_str(), data_port.c_str());
    mcast_socket = socket(AF_INET, SOCK_DGRAM, 0);



    //psize = 16;
    Input_management input_queue(psize, fsize*20);

    pthread_t listener;
    listening_thread_configuration thread_conf;//, &retransmision_requests};
    memset((void*)&thread_conf, 0, sizeof thread_conf);
    thread_conf.ctrl_port = ctrl_port;
    thread_conf.mcast_addr = mcast_addr;
    thread_conf.data_port = data_port;
    thread_conf.nazwa_stacji = nazwa_nadajnika;
    thread_conf.ret_list = &retransmision_requests;

    if(pthread_create(&listener, nullptr, listening_rexmit_lookup, (void*)&thread_conf)) {//(void *)&thread_conf
        printf("Error:unable to create thread");
        exit(1);
    }

    ROUND_TIMER timer(rtime);

    bool end_loop = true;
    while(end_loop) {
        input_queue.load_packs_from_input();
        emit_series_of_ordered_packages(mcast_socket, mcast_con, input_queue);
        if(timer.new_round_start()){
            packs_retransmission(mcast_socket, mcast_con, retransmision_requests, input_queue);
        }
    }
}

