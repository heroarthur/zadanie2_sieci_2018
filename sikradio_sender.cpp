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

#include "common/common.hpp"
#include "common/datagram_packing.hpp"

#include "sikradio_sender/audio_transmission.hpp"
#include "sikradio_sender/sikradio_listening.hpp"

using namespace std;


int main (int argc, char *argv[]) {
    string mcast_addr;
    string nazwa_nadajnika;
    string data_port;
    string ctrl_port;
    uint32_t psize;
    uint32_t fsize;
    uint32_t rtime;

    concurrent_uniqe_list<string> retransmision_requests;

    assign_sikradio_sender_default_arguments(nazwa_nadajnika, data_port,
                                             ctrl_port, psize, fsize, rtime);

    set_sikradio_sender_arguments(argc, argv, mcast_addr, nazwa_nadajnika,
                                  data_port, ctrl_port, psize, fsize, rtime);


    Input_management input_queue(psize, fsize);

    datagram_connection mcast_con{0,nullptr};
    initialize_mcast_connection(mcast_con, mcast_addr, data_port);

    pthread_t listener;

    listening_thread_configuration thread_conf{ctrl_port, mcast_addr, data_port,
                                               nazwa_nadajnika, &retransmision_requests};
    if(pthread_create(&listener, nullptr, listening_rexmit_lookup, (void *)&thread_conf)) {
        printf("Error:unable to create thread");
        exit(1);
    }


    bool end_loop = true;
    while(end_loop) {
        input_queue.load_packs_from_input();
        emit_series_of_ordered_packages(mcast_con, input_queue);
        if(retransmission_time(rtime))
            packs_retransmission(mcast_con, retransmision_requests, input_queue);
    }
}
