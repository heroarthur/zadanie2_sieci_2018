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


using namespace std;



bool next_retransmission_time(const uint32_t &rtime) {
    static clock_t last_check = clock();
    float diff = (clock() - last_check)*100;
    if(diff < rtime)
        return false;
    last_check = clock();
    return true;
}


void create_audio_pack(uint64_t session_id, byte_container &p, char *tr_pack, const ssize_t packs_size) {
    memcpy_number<uint64_t>(session_id, tr_pack, 0);
    memcpy_number<uint64_t>(p.first_byte_num, tr_pack, sizeof(uint64_t));
    p.write_to_array(tr_pack, sizeof(uint64_t)*2, packs_size);
}


void emit_series_of_ordered_packages(int sockfd, Connection_addres& con, Input_management &input_queue) {
    static auto* datagram = new char[2*sizeof(uint64_t) + input_queue.psize + 1]();//SO will clean it anyway
    static byte_container next_pack;

    while(input_queue.next_pack_available()) {
        input_queue.get_next_pack(next_pack);
        create_audio_pack(input_queue.session_id, next_pack, datagram, input_queue.psize);
        sendto_msg(sockfd, con, datagram, input_queue.audio_pack_size);
    }
}


void emit_single_package(int sockfd, Connection_addres& con, pack_id id, Input_management &input_queue) {
    static auto* datagram = new char[2*sizeof(uint64_t) + input_queue.psize + 1]();//SO will clean it anyway
    static byte_container pack;

    if(input_queue.pack_available(id)) {
        input_queue.get_pack(pack, id);
        create_audio_pack(input_queue.session_id, pack, datagram, input_queue.psize);
        sendto_msg(sockfd, con, datagram, input_queue.audio_pack_size);
    }
}


void packs_retransmission(int sockd, Connection_addres& con, concurrent_uniqe_list<string> &ret_list, Input_management &input_queue) {
    static std::list<string> ret_packs;
    ret_list.ret_uniqe_list(ret_packs);

    for (const pack_id &id : ret_packs) {
        emit_single_package(sockd, con, id, input_queue);
    }
}




//./zadanie2 -a "123.123.123.123" -P 1234 -n "spitfire nazwa odbiornika" -C 4444  -p 900 -f 300 -R 250
//write_sikradio_sender_arguments(mcast_addr, nazwa_odbiornika, data_port, ctrl_port, psize, fsize, rtime);
void write_sikradio_sender_arguments(string mcast_addr, string nazwa_nadajnika,
                                     string data_port, uint32_t ctrl_port,
                                     uint32_t psize, uint32_t fsize, uint32_t rtime) {
    printf("sikradio_sender arguments:\
    mcast_addr: %s\n\
    nazwa_nadajnika: %s\n\
    data_port: %s\n\
    ctrl_port: %u\n\
    psize: %u\n\
    fsize: %u\n\
    rtime: %u\n",
           mcast_addr.c_str(), nazwa_nadajnika.c_str(),
           data_port.c_str(), ctrl_port, psize, fsize, rtime);
}


void set_sikradio_sender_arguments(const int& argc, char **argv,
                                   string& mcast_addr, string& nazwa_nadajnika,
                                   string& data_port, string& ctrl_port,
                                   uint32_t& psize, uint32_t& fsize, uint32_t& rtime) {
    int c;
    opterr = 0;
    bool got_multicast_addres = false;

    while ((c = getopt(argc, argv, "a:d:p:b:n:f:P:C:R:")) != -1) {
        switch (c) {
            case 'a':
                mcast_addr = string(optarg);
                got_multicast_addres = true;
                break;
            case 'P':
                data_port = string(optarg);
                break;
            case 'n':
                nazwa_nadajnika = string(optarg);
                break;
            case 'C':
                ctrl_port = string(optarg);
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
    }

    if(!got_multicast_addres) {
        fprintf(stderr, "Pass multicast_addres with -a\n");
        exit(1);
    }
}


void assign_sikradio_sender_default_arguments(string& nazwa_nadajnika,
                                              string& data_port, string& ctrl_port,
                                              uint32_t& psize, uint32_t& fsize, uint32_t& rtime) {
    nazwa_nadajnika = NAZWA_DEF;
    data_port = to_string(DATA_PORT_DEF);
    ctrl_port = to_string(CTRL_PORT_DEF);
    psize = PSIZE_DEF;
    fsize = FSIZE_DEF;
    rtime = RTIME_DEF;
}