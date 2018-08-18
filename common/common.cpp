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
    static const string ZERO_SEVEN_COME_IN_text = "ZERO_SEVEN_COME_IN";
    return msg.substr(0, ZERO_SEVEN_COME_IN_text.length())
           == ZERO_SEVEN_COME_IN_text;
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
    if (((errno == ERANGE && (val <= ULONG_MAX)) ||
                         (errno != 0 && val == 0))) {
        printf("passed argument outside of range %c %s\n",
               (char)option, optarg);
        exit(1);
    }
    if (end == optarg) {
        printf("no digits were found %c %s\n", (char)option, optarg);
        exit(1);
    }
    if (end == optarg) {//*end == '\0' ||
        printf("argument have none-digit characters %c %s\n",
               (char)option, optarg);
        exit(1);
    }
    return (uint32_t)val;
}

bool validate_port(string& port) {
    const uint32_t MIN_PORT_NUM = 0;
    volatile uint32_t ret_val = parse_optarg_to_number(0, port.c_str());
    port = to_string(ret_val);
    return ret_val >= MIN_PORT_NUM;
}









void Input_management::load_packs_from_input() {
    byte_container readed_bytes = byte_container();
    static byte_container unfinished_pack = byte_container();
    read_input(readed_bytes);
    byte_container new_conteiner;

    while(readed_bytes.size() >= psize) {
        readed_bytes.pop_to_container(psize,
                                      next_unused_package_num, new_conteiner);
        dict.insert(to_string(next_unused_package_num), new_conteiner);
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
    uint64_t k = unrolled(max_key);
    return next_availabile_package_num <= k;
};


bool Input_management::pack_available(pack_id id) {
    return dict.contain(id);
}

void Input_management::read_input(byte_container& msg) {
    char read_buff[RECVFROM_BUFF_SIZE];

    ssize_t numbytes;
    numbytes = read(fileno(stdin), read_buff, RECVFROM_BUFF_SIZE);
    msg.emplace_back(read_buff, 0, numbytes);
}


void bind_socket(int& sockfd, Connection_addres& connection) {
    if (bind(sockfd, &connection.ai_addr, connection.ai_addrlen) == -1) {
        close(sockfd);
        perror("listener: bind");
        exit(1);
    }
}


void bind_to_first_free_port(int& sockfd) {
    Connection_addres con;
    string ip;
    for(uint32_t i = 100000; i < 65000; i++) {
        ip = to_string(i);
        get_communication_addr(con, USE_MY_IP, ip.c_str());
        if (bind(sockfd, &con.ai_addr, con.ai_addrlen) == -1) {
            continue;
        }
        break;
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






void fill_connection_struct(Connection_addres &connection,
                            struct addrinfo *servinfo) {
    connection.ai_addr = *(servinfo->ai_addr);
    connection.ai_addrlen = servinfo->ai_addrlen;
    connection.ai_family = servinfo->ai_family;
    connection.ai_socktype = servinfo->ai_socktype;
    connection.ai_protocol = servinfo->ai_protocol;
}


void get_communication_addr(Connection_addres& connection,
                            const char* ip_addr, const char* port) {
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
    freeaddrinfo(servinfo);
}



void receive_pending_messages(int& sockfd, list<recv_msg>& messages) {
    static char buff[RECVFROM_BUFF_SIZE];
    bool socket_clear = false;
    messages.clear();
    recv_msg m;
    struct sockaddr their_addr;
    socklen_t addr_len;

    ssize_t  numbytes;


    while(!socket_clear) {
        m = recv_msg{};
        if ((numbytes = recvfrom(sockfd, buff, RECVFROM_BUFF_SIZE-1,
                0, &their_addr, &addr_len)) == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                socket_clear = true;
                continue;
            }
            else {
                perror("recvfrom");
                continue;
            }
        }
        buff[numbytes] = '\0';
        m.text = string(buff);
        m.sender_addr.ai_addr = their_addr;
        m.sender_addr.ai_addrlen = addr_len;
        messages.emplace_back(m);
    }
}



void get_int64_bit_value(const char* datagram, uint64_t& val, int beg) {
    uint8_t a[8];
    mempcpy(a, datagram + beg, sizeof(uint64_t));
    for(uint32_t i = 0; i < sizeof(uint64_t); i++) {
        ((uint8_t*)&val)[i] = ((uint8_t*) a)[i];
    }
    val = be64toh(val);
}



void create_socket_binded_to_new_mcast_addr(int& mcast_sockfd,
                                            const char* mcast_addr,
                                            const char* data_port) {
    close(mcast_sockfd);
    mcast_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    Connection_addres myLocation{};
    get_communication_addr(myLocation, USE_MY_IP, data_port);
    bind_socket(mcast_sockfd, myLocation);
    fcntl(mcast_sockfd, F_SETFL, O_NONBLOCK);
    u_int enable=1;
    struct ip_mreq mreq{};
    mreq.imr_multiaddr.s_addr=inet_addr(mcast_addr);
    mreq.imr_interface.s_addr=htonl(INADDR_ANY);
    if (setsockopt(mcast_sockfd, SOL_SOCKET, SO_REUSEADDR,
                   &enable, sizeof(enable)) < 0) {
        perror("Reusing ADDR failed");
        exit(1);
    }
    if (setsockopt(mcast_sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                   &mreq, sizeof(mreq)) < 0) {
        perror("ssssetsockopt");
        exit(1);
    }
}



void sendto_msg(int& sockfd ,const Connection_addres& connection,
                const char* msg, const uint64_t msg_len) {
    ssize_t numbytes;
    if ((numbytes = sendto(sockfd, msg, msg_len, 0,
                           &(connection.ai_addr),
                           connection.ai_addrlen)) == -1) {
        perror("talker: sendto");
        exit(1);
    }
}



void sendto_msg(int& sockfd ,Connection_addres connection, const char* msg,
                const uint64_t msg_len, uint16_t destination_port) {
    ssize_t numbytes;
    //((sockaddr_in*)&connection)->sin_port = destination_port;
        //msg[msg_len] = '\0';
        if ((numbytes = sendto(sockfd, msg, msg_len, 0,
                           &(connection.ai_addr),
                           connection.ai_addrlen)) == -1) {
        perror("talker: sendto");
        exit(1);
    }
}


inline uint64_t unrolled(std::string const& value) {
    uint64_t result = 0;

    size_t const length = value.size();
    switch (length) {
        case 20:    result += (value[length - 20] - '0') * 10000000000000000000ULL;
        case 19:    result += (value[length - 19] - '0') * 1000000000000000000ULL;
        case 18:    result += (value[length - 18] - '0') * 100000000000000000ULL;
        case 17:    result += (value[length - 17] - '0') * 10000000000000000ULL;
        case 16:    result += (value[length - 16] - '0') * 1000000000000000ULL;
        case 15:    result += (value[length - 15] - '0') * 100000000000000ULL;
        case 14:    result += (value[length - 14] - '0') * 10000000000000ULL;
        case 13:    result += (value[length - 13] - '0') * 1000000000000ULL;
        case 12:    result += (value[length - 12] - '0') * 100000000000ULL;
        case 11:    result += (value[length - 11] - '0') * 10000000000ULL;
        case 10:    result += (value[length - 10] - '0') * 1000000000ULL;
        case  9:    result += (value[length -  9] - '0') * 100000000ULL;
        case  8:    result += (value[length -  8] - '0') * 10000000ULL;
        case  7:    result += (value[length -  7] - '0') * 1000000ULL;
        case  6:    result += (value[length -  6] - '0') * 100000ULL;
        case  5:    result += (value[length -  5] - '0') * 10000ULL;
        case  4:    result += (value[length -  4] - '0') * 1000ULL;
        case  3:    result += (value[length -  3] - '0') * 100ULL;
        case  2:    result += (value[length -  2] - '0') * 10ULL;
        case  1:    result += (value[length -  1] - '0');
    }
    return result;
}


