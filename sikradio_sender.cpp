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

#include "common.hpp"

using namespace std;


//./zadanie2 -a "123.123.123.123" -P 1234 -n "spitfire nazwa odbiornika" -C 4444  -p 900 -f 300 -R 250
//write_sikradio_sender_arguments(mcast_addr, nazwa_odbiornika, data_port, ctrl_port, psize, fsize, rtime);
void write_sikradio_sender_arguments(string mcast_addr, string nazwa_odbiornika,
                                     string data_port, uint32_t ctrl_port,
                                     uint32_t psize, uint32_t fsize, uint32_t rtime) {
    printf("sikradio_sender arguments:\
    mcast_addr: %s\n\
    nazwa_odbiornika: %s\n\
    data_port: %s\n\
    ctrl_port: %u\n\
    psize: %u\n\
    fsize: %u\n\
    rtime: %u\n",
           mcast_addr.c_str(), nazwa_odbiornika.c_str(),
           data_port.c_str(), ctrl_port, psize, fsize, rtime);
}


void set_sikradio_sender_arguments(const int& argc, char **argv,
                           string& mcast_addr, string& nazwa_odbiornika,
                           string& data_port, uint32_t& ctrl_port,
                           uint32_t& psize, uint32_t& fsize, uint32_t& rtime) {
    int c;
    opterr = 0;

    while ((c = getopt(argc, argv, "a:d:p:b:n:f:P:C:R:")) != -1)
        switch (c) {
            case 'a':
                mcast_addr = string(optarg);
                break;
            case 'P':
                data_port = string(optarg);
                break;
            case 'n':
                nazwa_odbiornika = string(optarg);
                break;
            case 'C':
                ctrl_port = parse_optarg_to_number('C', optarg);
                break;
            case 'p':
                psize = parse_optarg_to_number('p', optarg);
                break;
            case 'f':
                fsize = parse_optarg_to_number('f', optarg);
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
    /*
    if (argv[optind] == nullptr || argv[optind + 1] == nullptr) {
        printf("Mandatory argument(s) missing\n");
        exit(1);
    }
    */
}


/*
void send_input_to_multicast(const string& addres, const string& data_port, const uint32_t psize,
                             Input_fifo_queue& input_queue) {
    static bool socket_initialized = false;
    static int sockfd;
    static struct addrinfo hints, *servinfo, *p;
    static int rv;
    static int numbytes;

    if(!socket_initialized) {
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_DGRAM;

        if ((rv = getaddrinfo(addres.c_str(), data_port.c_str(), &hints, &servinfo)) != 0) {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
            exit(1);
        }
        // loop through all the results and make a socket
        for(p = servinfo; p != NULL; p = p->ai_next) {
            if ((sockfd = socket(p->ai_family, p->ai_socktype,
                                 p->ai_protocol)) == -1) {
                perror("talker: socket");
                continue;
            }

            break;
        }
        if (p == NULL) {
            fprintf(stderr, "talker: failed to create socket\n");
            exit(1);
        }
        freeaddrinfo(servinfo);
    }

    if ((numbytes = sendto(sockfd, argv[2], sizeof(char)*psize, 0,
                           p->ai_addr, p->ai_addrlen)) == -1) {
        perror("talker: sendto");
        exit(1);
    }



    printf("talker: sent %d bytes to %s\n", numbytes, argv[1]);
    close(sockfd);




}
*/


void do_package_retransmission() {


    exit(1);
}


struct retransmission_request {
    string packages;
    struct sockaddr_storage retr_addr;
};


list<retransmission_request> retransmision_requests;
//Input_packs_queue input_queue;

int main (int argc, char *argv[]) {
    string mcast_addr;
    string nazwa_odbiornika;
    string data_port;
    uint32_t ctrl_port;
    uint32_t psize;
    uint32_t fsize;
    uint32_t rtime;

    set_sikradio_sender_arguments(argc, argv, mcast_addr, nazwa_odbiornika,
                                  data_port, ctrl_port, psize, fsize, rtime);




    while(true) {
        //input_queue.read_input();
        //send_input_to_multicast(input_queue);
        do_package_retransmission();
    }

}
