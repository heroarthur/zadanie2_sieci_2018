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


#include "common/common.hpp"

#include "sikradio_receiver/receive_managment.hpp"

using namespace std;




int main_ (int argc, char *argv[]) {
    string discover_addr;
    string ui_port;
    string ctrl_port;
    uint32_t bsize;
    uint32_t rtime;

    set_sikradio_receiver_arguments(argc, argv, discover_addr, ui_port, ctrl_port, bsize, rtime);
    transmitters_set finded_transmitters;

    radio_receiver receiver = radio_receiver(discover_addr, ui_port, ctrl_port, bsize, rtime);


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
    fcntl(recv_senders_id, F_SETFL, O_NONBLOCK);
    fcntl(recv_senders_id, F_SETFL, O_ASYNC);



    while(true) {
        broadcast_lookup(broadcast_sockfd, broadcast_location,
                         ZERO_SEVEN_COME_IN.c_str(), ZERO_SEVEN_COME_IN.length());
        receive_senders_identyfication(recv_senders_id, finded_transmitters);
        //receiver.manage_audio_package();
    }
}
