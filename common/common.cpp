//
// Created by karol on 21.07.18.
//
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
#include<climits>
#include <queue>

#include "common.hpp"

using namespace std;

bool msgIstring (string msg) {
    return msg.substr(0, ZERO_SEVEN_COME_IN.length()) == ZERO_SEVEN_COME_IN;
}
bool msgIsRexmit(string msg) {
    return msg.substr(0, LOUDER_PLEASE.length()) == LOUDER_PLEASE;
}
bool msgIsBorewicz(string msg) {
    return msg.substr(0, BOREWICZ_HERE.length()) == BOREWICZ_HERE;
}




uint32_t parse_optarg_to_number(int option, char *optarg) {
    char *end;
    constexpr int decimal = 10;
    uint64_t val = std::strtoul(optarg, &end, decimal);
    if ((errno == ERANGE && (val <= ULONG_MAX) || (errno != 0 && val == 0))) {
        printf("passed argument outside of range %c %s\n", (char)option, optarg);
        exit(1);
    }
    if (end == optarg) {
        printf("no digits were found %c %s\n", (char)option, optarg);
        exit(1);
    }
    if (*end == '\0' || end == optarg) {
        printf("argument have none-digit characters %c %s\n", (char)option, optarg);
        exit(1);
    }
    return (uint32_t)val;
}


void Input_management::load_packs_from_input() {
    static char tmp[128000];
    static string unfinished_pack;
    static uint32_t pack_start;
    read_input(tmp);
    string s = unfinished_pack + string(tmp);
    for(pack_start=0; pack_start+psize <= s.length(); pack_start += psize) {
        dict.insert(to_string(next_package_num),
                          packgs{next_package_num, s.substr(pack_start, psize)});
        next_package_num += psize;
    }
    unfinished_pack = s.substr(pack_start, psize);
}

void Input_management::get_next_pack(packgs& p) {
    while(!dict.contain(to_string(next_package_num))) {
        next_package_num += psize;
    }
    p = dict.get(to_string(next_package_num));
    next_package_num += psize;
}

void Input_management::get_pack(packgs& p, pack_id id) {
    p = dict.get(id);
}

bool Input_management::next_pack_available() {
    if(dict.empty()) return false;
    string max_key = dict.last_inserted_key();
    std::istringstream iss(max_key);
    uint64_t k; iss >> k;
    return next_package_num <= k;
};


bool Input_management::pack_available(pack_id id) {
    return dict.contain(id);
}

void Input_management::read_input(char* buff) {
    memset(buff, 0, INPUT_READ_SIZE);
    read(0, buff, INPUT_READ_SIZE);
}





void recv_msg_from(string& recv_msg, const Connection& connection) {
    static const uint32_t buff_size = 10000;
    static char buff[buff_size];
    memset(buff, 0, buff_size);
    if (recvfrom(connection.sockfd, buff, buff_size, 0,
                 (struct sockaddr *)&connection.send_addr,
                 &connection.addr_len)) {
        perror("recvfrom");
        exit(1);
    }
    recv_msg = string(buff);
}


void sendto_msg(Connection& connection, const string& msg) {
    ssize_t numbytes;
    if ((numbytes = sendto(connection.sockfd, msg.c_str(), msg.length(), 0,
                           (struct sockaddr *)&connection.send_addr,
                           connection.addr_len)) == -1) {
        perror("talker: sendto");
        exit(1);
    }
    if(numbytes != msg.length()) {
        printf("could't send whole message");
        exit(1);
    }
}


void create_datagram_socket(Connection& new_connection, const string& port, string* ip) {
    int rv;
    ssize_t numbytes;
    struct addrinfo hints, *servinfo, *p;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo(ip != nullptr ? (*ip).c_str():nullptr , port.c_str(), &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }
    // loop through all the results and make a socket
    for(p = servinfo; p != nullptr; p = p->ai_next) {
        if ((new_connection.sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            perror("talker: socket");
            continue;
        }
        break;
    }
    if (p == nullptr) {
        fprintf(stderr, "talker: failed to create socket\n");
        exit(1);
    }
    new_connection.send_addr = *(sockaddr_storage*)p->ai_addr;
    new_connection.addr_len = p->ai_addrlen;
}

void create_mcast_listeninig_socket(Connection& new_connection, const string mcast_group, const string port) {
    create_datagram_socket(new_connection, port, CHOOSE_MY_IP);
    u_int yes=1;
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr=inet_addr(mcast_group.c_str());
    mreq.imr_interface.s_addr=htonl(INADDR_ANY);
    if (setsockopt(new_connection.sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes)) < 0) {
        perror("Reusing ADDR failed");
        exit(1);
    }
    if (setsockopt(new_connection.sockfd,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreq,sizeof(mreq)) < 0) {
        perror("setsockopt");
        exit(1);
    }
}







