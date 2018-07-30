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
#include "receive_managment.hpp"


void sendto_msg(const int& sockfd, const addrinfo& send_addr, const string& msg) {
    ssize_t numbytes;
    if ((numbytes = sendto(sockfd, msg.c_str(), msg.length(), 0,
                           send_addr.ai_addr, send_addr.ai_addrlen)) == -1) {
        perror("talker: sendto");
        exit(1);
    }
    if(numbytes != msg.length()) {
        printf("could't send whole message");
        exit(1);
    }
    close(sockfd);
}


void create_datagram_socket(int &sockfd, addrinfo &sendto_addr, const string ip_addr, const string port) {
    int rv;
    ssize_t numbytes;
    struct addrinfo hints, *servinfo, *p;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo(ip_addr.c_str(), port.c_str(), &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }
    // loop through all the results and make a socket
    for(p = servinfo; p != nullptr; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
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
}


void radio_receiver::send_lookup() {
    if(true) {//minal rtime
        sendto_msg(lookup_sockfd, lookup_addr, LOOKUP);
    }
}


void radio_receiver::receive_senders_identyfication() {
    //dopoki mozesz odbieraj z non blocking socket


}















































































































































//write_sikradio_receiver_arguments(ui_port, ctrl_port, bsize, rtime);
void write_sikradio_receiver_arguments(uint32_t ui_port, uint32_t ctrl_port,
                                       uint32_t bsize, uint32_t rtime) {
    printf("sikradio_receiver arguments:\n\
    ui_port: %u\n\
    ctrl_port: %u\n\
    bsize: %u\n\
    rtime: %u\n", ui_port, ctrl_port, bsize, rtime);
}

void set_sikradio_receiver_arguments(const int& argc, char **argv,
                                     string& discover_addr, string& ui_port, string& ctrl_port,
                                     uint32_t& bsize, uint32_t& rtime) {
    int c;
    opterr = 0;

    while ((c = getopt(argc, argv, "d:b:U:C:R:")) != -1)
        switch (c) {
            case 'd':
                discover_addr = string(optarg);
                break;
            case 'b':
                bsize = parse_optarg_to_number('b', optarg);
                break;
            case 'U':
                ui_port = string(optarg);
                break;
            case 'C':
                ctrl_port = string(optarg);
                break;
            case 'R':
                rtime = parse_optarg_to_number('R', optarg);
                break;

            case '?':
                if (optopt == 'c')
                    fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                else if (isprint(optopt))
                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                else
                    fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
                exit(1);
            default:
                abort();
        }

    if (argv[optind] == nullptr || argv[optind + 1] == nullptr) {
        printf("Mandatory argument(s) missing\n");
        exit(1);
    }
}