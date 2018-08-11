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

#include <ctime>
#include <sys/time.h>

#include "common.hpp"

using namespace std;

bool msgIsLookup(string msg) {
    return msg.substr(0, ZERO_SEVEN_COME_IN.length()) == ZERO_SEVEN_COME_IN;
}
bool msgIsRexmit(string msg) {
    return msg.substr(0, LOUDER_PLEASE.length()) == LOUDER_PLEASE;
}
bool msgIsBorewicz(string msg) {
    return msg.substr(0, BOREWICZ_HERE.length()) == BOREWICZ_HERE;
}




uint32_t parse_optarg_to_number(int option, const char *optarg) {
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
    if (end == optarg) {//*end == '\0' ||
        printf("argument have none-digit characters %c %s\n", (char)option, optarg);
        exit(1);
    }
    return (uint32_t)val;
}


void Input_management::load_packs_from_input() {
    byte_container readed_bytes = byte_container();
    static byte_container unfinished_pack = byte_container();
    static uint32_t pack_start;
    read_input(readed_bytes);//emplace back wszystkie wczytane bajty

    while(readed_bytes.size() >= psize) {
        dict.insert(to_string(next_unused_package_num), readed_bytes.pop_to_container(psize, next_unused_package_num));
        next_unused_package_num += psize;
    }
}

void Input_management::get_next_pack(byte_container& p) {
    while(!dict.contain(to_string(next_availabile_package_num))) {
        next_availabile_package_num += psize;
    }
    p = dict.get(to_string(next_availabile_package_num));
    next_availabile_package_num += psize;
}

void Input_management::get_pack(byte_container& p, pack_id id) {
    p = dict.get(id);
}

bool Input_management::next_pack_available() {
    if(dict.empty()) return false;
    string max_key = dict.last_inserted_key();
    std::istringstream iss(max_key);
    uint64_t k; iss >> k;
    return next_availabile_package_num <= k;
};


bool Input_management::pack_available(pack_id id) {
    return dict.contain(id);
}

void Input_management::read_input(byte_container& msg) {
    static ROUND_TIMER t(250);
    //while(!t.new_round_start()) {}

    char arr[16] = "go_spitfireeeee";
    msg.emplace_back(arr, 0, 16);
    //memset(msg.buff, 0, INPUT_READ_SIZE);
    //read(0, msg.buff, INPUT_READ_SIZE);
    //fgets(buff, 512, (FILE*)stdin_debug_fd);
}


void bind_socket(int& sockfd, Connection_addres& connection) {
    if (bind(sockfd, &connection.ai_addr, connection.ai_addrlen) == -1) {
        close(sockfd);
        perror("listener: bind");
        exit(1);
    }
}


uint64_t current_time_sec() {
    static struct timeval tval;
    if(gettimeofday(&tval, nullptr) == 0) {
        uint64_t current_time_sec = (uint64_t)tval.tv_sec;
        return current_time_sec;
    }
    return 0;
}






void fill_connection_struct(Connection_addres &connection, struct addrinfo *servinfo) {
    connection.ai_addr = *(servinfo->ai_addr);
    //memcpy ( (void *) &connection.ai_addr, (const void *)(sockaddr_storage*)(servinfo->ai_addr), sizeof(sockaddr_storage));
    //memcpy(&connection.ai_addr, &(servinfo->ai_addr), sizeof(sockaddr));
    connection.ai_addrlen = servinfo->ai_addrlen;
    connection.ai_family = servinfo->ai_family;
    connection.ai_socktype = servinfo->ai_socktype;
    connection.ai_protocol = servinfo->ai_protocol;
}


void get_communication_addr(Connection_addres& connection, const char* ip_addr, const char* port) {
    int rv;
    struct addrinfo hints, *servinfo;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // set to AF_INET to force IPv4
    hints.ai_socktype = SOCK_DGRAM;

    if(ip_addr == USE_MY_IP)
        hints.ai_flags = AI_PASSIVE; // use my IP
    if ((rv = getaddrinfo(ip_addr, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }
    fill_connection_struct(connection, servinfo);
    freeaddrinfo(servinfo); //sprawdz z brake czy wartosci sie skopiowaly poprawnie
}


void receive_pending_messages(int& sockfd, list<recv_msg>& messages) {
    static char buff[RECVFROM_BUFF_SIZE];
    bool socket_clear = false;
    messages.clear();
    recv_msg m;
    struct sockaddr their_addr;
    socklen_t addr_len;


    while(!socket_clear) {
        m = recv_msg{};
        //memset(buff, 0, sizeof buff);
        if (recvfrom(sockfd, buff, RECVFROM_BUFF_SIZE-1 , 0, &their_addr, &addr_len) == -1) {
        //if (recvfrom(sockfd, buff, RECVFROM_BUFF_SIZE-1 , 0, &m.sender_addr.ai_addr, &m.sender_addr.ai_addrlen) == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                socket_clear = true;
                continue;
            }
            else {
                perror("recvfrom");
                continue;
            }
        }
        m.text = string(buff);
        m.sender_addr.ai_addr = their_addr;
        m.sender_addr.ai_addrlen = addr_len;
        //printf("%s \n", m.text.c_str());
        messages.emplace_back(m);
    }
}


void create_socket_binded_to_new_mcast_addr(int& mcast_sockfd, const char* mcast_addr, const char* data_port) {
    Connection_addres myLocation{};
    get_communication_addr(myLocation, USE_MY_IP, data_port);
    mcast_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    bind_socket(mcast_sockfd, myLocation);
    fcntl(mcast_sockfd, F_SETFL, O_NONBLOCK);
    u_int enable=1;
    struct ip_mreq mreq{};
    mreq.imr_multiaddr.s_addr=inet_addr(mcast_addr);
    mreq.imr_interface.s_addr=htonl(INADDR_ANY);
    if (setsockopt(mcast_sockfd,SOL_SOCKET,SO_REUSEADDR,&enable,sizeof(enable)) < 0) {
        perror("Reusing ADDR failed");
        exit(1);
    }
    if (setsockopt(mcast_sockfd,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreq,sizeof(mreq)) < 0) {
        perror("setsockopt");
        exit(1);
    }
}



void sendto_msg(int& sockfd ,const Connection_addres& connection, const char* msg, const uint64_t msg_len) {
    ssize_t numbytes;
    if ((numbytes = sendto(sockfd, msg, msg_len, 0,
                           &(connection.ai_addr),
                           connection.ai_addrlen)) == -1) {
        perror("talker: sendto");
        printf("spitfire1\n");
        exit(1);
    }
}



void sendto_msg(int& sockfd ,Connection_addres connection, const char* msg, const uint64_t msg_len, uint16_t destination_port) {
    ssize_t numbytes;
    ((sockaddr_in*)&connection)->sin_port = destination_port;
    if ((numbytes = sendto(sockfd, msg, msg_len, 0,
                           &(connection.ai_addr),
                           connection.ai_addrlen)) == -1) {
        perror("talker: sendto");
        printf("spitfire2\n");
        exit(1);
    }
}




