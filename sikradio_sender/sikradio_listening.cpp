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


const ssize_t listener_buff_size = 1000;
//ctrl_port nasluchuje na rexmit i ZERO_SEVEN_COME_IN
//data_port na tym porcie receiver ma nasluchiwac na pakiety audio





string reply_communicat(string header, string mcast_addr, string data_port, string nazwa_stacji) {
    //BOREWICZ_HERE [MCAST_ADDR] [DATA_PORT] [nazwa stacji]
    const string del = " ";
    string reply = header + del + \
            mcast_addr + del \
            + data_port + del \
            + nazwa_stacji + del;
    return reply;
}


list<string> rexmit_to_list(string rexmit) {
    string parse_text = rexmit.substr(LOUDER_PLEASE.length()+1);
    string delim = ",";
    uint64_t start = 0U;
    uint64_t end = rexmit.find(delim);
    std::list<string> l;
    while (end != string::npos)
    {
        l.emplace_back(parse_text.substr(start, end - start));
        start = end + delim.length();
        end = parse_text.find(delim, start);
    }
    return l;
}


void *listening_rexmit_lookup(void *thread_data) {
    listening_thread_configuration* config = (listening_thread_configuration*)thread_data;
    string ctrl_port = config->ctrl_port;
    uint16_t ctrl_port_int = htons((uint16_t)parse_optarg_to_number(0, ctrl_port.c_str()));


    string mcast_addr = config->mcast_addr;
    string data_port = config->data_port;

    string nazwa_stacji = config->nazwa_stacji;
    concurrent_uniqe_list<string> *rexmit_requests_list = (config->ret_list);
    string reply_msg = reply_communicat(BOREWICZ_HERE, mcast_addr, data_port, nazwa_stacji);

    int rexmit_lookup_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    Connection_addres rexmit_lookup_addr{};
    get_communication_addr(rexmit_lookup_addr, USE_MY_IP, ctrl_port.c_str());
    bind_socket(rexmit_lookup_sockfd, rexmit_lookup_addr);
    int l = 1;
    setsockopt(rexmit_lookup_sockfd, SOL_SOCKET, SO_REUSEADDR, &l, sizeof(int));

    int reply_identyfication_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    Connection_addres requester_addr;

    char buff[RECVFROM_BUFF_SIZE];
    string recv_msg;

    while(true) {
        if (recvfrom(rexmit_lookup_sockfd, buff, RECVFROM_BUFF_SIZE-1 , 0,
                                 &requester_addr.ai_addr, &requester_addr.ai_addrlen) == -1) {
            perror("recvfrom");
            exit(1);
        }
        recv_msg = string(buff);
        printf("%s \n", recv_msg.c_str());
        if(msgIsLookup(recv_msg)) {
            //    their_addr.sin_port = htons(CTRL_PORT_DEF); // short, network byte order
            sendto_msg(reply_identyfication_sockfd, requester_addr, reply_msg.c_str(), reply_msg.length(), ctrl_port_int);
        }
        if(msgIsRexmit(recv_msg)) {//odpal tu function template zeby dalo sie i liste i vector
            printf("rexmit_msg %s \n", recv_msg.c_str());
            std::list<string> l = split_string_to_container<std::list<string>>
                    (recv_msg.substr(LOUDER_PLEASE.length() + 1), ",");
            (*rexmit_requests_list).insert(l);
        }
    }
}