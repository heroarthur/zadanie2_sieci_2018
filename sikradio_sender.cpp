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

using namespace std;


int main (int argc, char *argv[]) {
    string mcast_addr;
    string nazwa_odbiornika;
    string data_port;
    uint32_t ctrl_port;
    uint32_t psize;
    uint32_t fsize;
    uint32_t rtime;

    concurrent_uniqe_list<string> retransmision_requests;

    assign_sikradio_sender_default_arguments(nazwa_odbiornika, data_port,
                                             ctrl_port, psize, fsize, rtime);

    set_sikradio_sender_arguments(argc, argv, mcast_addr, nazwa_odbiornika,
                                  data_port, ctrl_port, psize, fsize, rtime);


    Input_management input_queue(psize, fsize);

    datagram_connection mcast_con{0,nullptr};
    initialize_mcast_connection(mcast_con, mcast_addr, data_port);

    int mcast_sockfd;
    struct addrinfo *p;
    //pthread odpal nasluchiwanie

    while(true) {
        input_queue.load_packs_from_input();
        emit_series_of_ordered_packages(mcast_con, input_queue);
        if(retransmission_time(rtime))
            packs_retransmission(mcast_con, retransmision_requests, input_queue);
    }
}
