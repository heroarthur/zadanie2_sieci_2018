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

#include "common/common.hpp"

#include "sikradio_receiver/receive_managment.hpp"

using namespace std;



bool message_pending(int& fd, fd_set& readfds) {
    static struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    select(fd+1, &readfds, nullptr, nullptr, &tv);

    if (FD_ISSET(fd, &readfds))
        return true;
    return false;
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
    fcntl(recv_senders_id, F_SETFL, O_NONBLOCK);



    struct sockaddr their_addr;
    socklen_t addr_len;
    fd_set readfds;

    FD_ZERO(&readfds);
    FD_SET(recv_senders_id, &readfds);

    // don't care about writefds and exceptfds:



    while(true) {
        broadcast_lookup<5000>(broadcast_sockfd, broadcast_location,
                        ZERO_SEVEN_COME_IN.c_str(), ZERO_SEVEN_COME_IN.length());
        if(message_pending(recv_senders_id, readfds))
            receive_senders_identyfication(recv_senders_id, finded_transmitters);
        manage_receive_audio(send_rexmit_sockfd, ctrl_port_num, bsize, finded_transmitters, rtime);
        //clear_not_reported_transmitters(finded_transmitters);
    }
}
