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
//#include "../common/datagram_packing.hpp"

#include "audio_transmission.hpp"
#include "sikradio_listening.hpp"

using namespace std;


string reply_communicat(string header, string mcast_addr, string data_port,
                                                    string nazwa_stacji) {
    //BOREWICZ_HERE [MCAST_ADDR] [DATA_PORT] [nazwa stacji]
    string del = " ";
    string reply_msg = header + del +
            mcast_addr + del +
            data_port + del +
            nazwa_stacji;
    return reply_msg;
}





void *listening_rexmit_lookup(void *thread_data) {
     listening_thread_configuration* config =
             (listening_thread_configuration*)thread_data;
     string ctrl_port = config->ctrl_port;

     string mcast_addr = config->mcast_addr;
     string data_port = config->data_port;

     string nazwa_stacji = config->nazwa_stacji;
     concurrent_uniqe_list<string> *rexmit_requests_list = (config->ret_list);

     string reply_msg = reply_communicat(BOREWICZ_HERE, mcast_addr,
             data_port, nazwa_stacji);

     int recv_send_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
     Connection_addres rexmit_lookup_addr{};
     get_communication_addr(rexmit_lookup_addr, USE_MY_IP,
                            ctrl_port.c_str());
     bind_socket(recv_send_sockfd, rexmit_lookup_addr);
     int l = 1;
     setsockopt(recv_send_sockfd, SOL_SOCKET, SO_REUSEADDR, &l, sizeof(int));

     int reply_identyfication_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
     Connection_addres requester_addr{};
     requester_addr.ai_addrlen = sizeof(sockaddr);

     char buff[RECVFROM_BUFF_SIZE];
     string recv_msg;

     while(true) {


         if (recvfrom(recv_send_sockfd, buff, RECVFROM_BUFF_SIZE-1 , 0, &requester_addr.ai_addr, &requester_addr.ai_addrlen) == -1) {
             perror("recvfrom");
             exit(1);
         }
         recv_msg = string(buff);
         if(msgIsLookup(recv_msg)) {
             sendto_msg(reply_identyfication_sockfd, requester_addr,
                        reply_msg.c_str(), reply_msg.length()+1);
         }
         if(msgIsRexmit(recv_msg)) {
             auto l = split_string_to_container<std::list<string>>
                     (recv_msg.substr(LOUDER_PLEASE.length() + 1), ",");
             (*rexmit_requests_list).insert(l);
         }
     }
}
