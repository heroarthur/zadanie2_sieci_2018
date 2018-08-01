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
#include "../common/datagram_packing.hpp"

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

void borewicz_here(int sockfd, sockaddr_storage their_addr, socklen_t addr_len, string borewicz_reply) {
    int numbytes;
    if (numbytes = (sendto(sockfd, borewicz_reply.c_str(), borewicz_reply.length(), 0,
                           (struct sockaddr *)&their_addr, addr_len)) == -1) {
        perror("talker: sendto");
        exit(1);
    }
    if(numbytes != borewicz_reply.length()) {
        perror("talker: sendto send only part of message");
        exit(1);
    }
}




list rexmit_to_list(string rexmit) {
    string parse_text = rexmit.substr(LOUDER_PLEASE.length()+1);
    string delim = ",";
    uint64_t start = 0U;
    uint64_t end = rexmit.find(delim);
    std::list l;
    while (end != string::npos)
    {
        l.emplace_back(parse_text.substr(start, end - start));
        start = end + delim.length();
        end = parse_text.find(delim, start);
    }
    return l;
}



void bind_rexmit_lookup_listener(int& sockfd, string ctrl_port) {
    int rv;
    struct addrinfo hints, *servinfo, *p;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(nullptr, ctrl_port.c_str(), &hints, &servinfo)) != 0) {
        fprintf(stderr, "rexmit/ZERO_SEVEN_COME_IN thread listener getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }
    // loop through all the results and bind to the first we can
    for(p = servinfo; p != nullptr; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            perror("listener: socket");
            continue;
        }
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("listener: bind");
            continue;
        }
        break;
    }
    if (p == nullptr) {
        fprintf(stderr, "listener: failed to bind socket\n");
        exit(2);
    }
    freeaddrinfo(servinfo);
}


void *listening_rexmit_lookup(void *thread_data) {
    listening_thread_configuration* config = (listening_thread_configuration*)thread_data;
    string ctrl_port = config->ctrl_port;
    string mcast_addr = config->mcast_addr;
    string data_port = config->data_port;
    string nazwa_stacji = config->data_port;
    concurrent_uniqe_list *rexmit_requests_list = config->ret_list;
    string reply_msg = reply_communicat(BOREWICZ_HERE, mcast_addr, data_port, nazwa_stacji);

    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    ssize_t numbytes;
    struct sockaddr_storage their_addr;
    char buf[listener_buff_size];
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];

    bind_rexmit_lookup_listener(sockfd, ctrl_port);
    addr_len = sizeof their_addr;
    string recv_msg;
    while(true) {
        if ((numbytes = recvfrom(sockfd, buf, listener_buff_size-1 , 0,
                                 (struct sockaddr *)&their_addr, &addr_len)) == -1) {
            perror("recvfrom");
            exit(1);
        }
        recv_msg = string(buf);
        if(isLookup(recv_msg)) {
            borewicz_here(sockfd, their_addr, addr_len, reply_msg);
        }
        if(msgIsRexmit(recv_msg)) {//odpal tu function template zeby dalo sie i liste i vector
            rexmit_requests_list->insert(
                    split_string_to_container<std::list>
                            (recv_msg.substr(LOUDER_PLEASE.length() + 1), ","));
        }
    }




}