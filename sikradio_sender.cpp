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
#include "datagram_packing.hpp"


using namespace std;



//typedef list<retransmission_request> request_list;
struct retransmission_request {
    bool operator==(const retransmission_request &other) const
    { return memcmp((const void *)this, (const void *)other,
                    sizeof(retransmission_request)) == 0;
    }
    string packages;
    struct sockaddr_storage retr_addr;
};

typedef list<string> packsToRetransmission;

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


void create_audio_pack(uint64_t session_id, packgs &p, char *tr_pack) {
    memset(tr_pack, 0, sizeof(tr_pack));
    strcat_number(session_id, tr_pack);
    strcat_number(p.first_byte, tr_pack);
    strcat(const_cast<char *>(p.bytes.c_str()), tr_pack);
}


void send_datagram(int &sockfd, addrinfo *p, char* datagram, uint32_t size) {
    static ssize_t numbytes;
    if ((numbytes = sendto(sockfd, datagram, size, 0,
                           p->ai_addr, p->ai_addrlen)) == -1) {
        perror("talker: sendto");
        exit(1);
    }
}


void emit_series_of_ordered_packages(int &sockfd, addrinfo *p, Input_management &input_queue) {
    static auto* datagram = new char[2*sizeof(uint64_t) + input_queue.psize + 1]();//SO will clean it anyway
    static packgs next_pack;

    while(input_queue.next_pack_available()) {
        input_queue.get_next_pack(next_pack);
        create_audio_pack(input_queue.session_id, next_pack, datagram);
        send_datagram(sockfd, p, datagram, input_queue.audio_pack_size);
    }
}



void emit_single_package(int &sockfd, addrinfo *p, pack_id id, Input_management &input_queue) {
    static auto* datagram = new char[2*sizeof(uint64_t) + input_queue.psize + 1]();//SO will clean it anyway
    static packgs pack;

    if(input_queue.pack_available(id)) {
        input_queue.get_pack(pack, id);
        create_audio_pack(input_queue.session_id, pack, datagram);
        send_datagram(sockfd, p, datagram,  input_queue.audio_pack_size);
    }
}


void packs_retransmission(int &sockfd, addrinfo *p, packsToRetransmission &packs, Input_management &input_queue) {
    packs.unique();
    packs.sort();
    for (const pack_id &id : packs) {
        emit_single_package(sockfd, p, id, input_queue);
    }
}




void initialize_mcast_connection(int& sockfd, addrinfo* p,
                                 const string &addres, const string &data_port){
    static int rv;
    static int numbytes;
    struct packgs next_pack;
    static struct addrinfo hints, *servinfo;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo(addres.c_str(), data_port.c_str(), &hints, &servinfo)) != 0) {
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
    freeaddrinfo(servinfo);
}









packsToRetransmission retransmision_requests;







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


    Input_management input_queue(psize, fsize);


    int mcast_sockfd;
    struct addrinfo *p;
    initialize_mcast_connection(mcast_sockfd, p, mcast_addr, data_port);
    //pthread odpal nasluchiwanie

    while(true) {
        input_queue.load_packs_from_input();
        emit_series_of_ordered_packages(mcast_sockfd, p, input_queue);
        if(false)//nalezy zrobic retransmisje
            packs_retransmission(mcast_sockfd, p, retransmision_requests, input_queue);
    }
}
