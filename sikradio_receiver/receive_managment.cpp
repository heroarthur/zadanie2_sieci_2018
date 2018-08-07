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





void broadcast_lookup(int sockfd, Connection_addres broadcast_addr, const char* msg, const uint64_t msg_len) {
    sendto_msg(sockfd, broadcast_addr, msg, msg_len);
}


void parse_identyfication(const recv_msg& identyfication, transmitter_addr& transmitter) {
    vector<string> l = split_string_to_container(identyfication.text, " ");
    transmitter = {};
    transmitter.mcast_addr = l[second];
    transmitter.data_port = l[third];
    transmitter.nazwa_stacji = join_container_elements<std::vector, from_fourth>(l, " ");
    time(&transmitter.last_reported);
    transmitter.direct_rexmit_con = identyfication.sender_addr;
}


void update_sender_identyfication(const recv_msg& identyfication, transmitters_set& transmitters) {
    if(!msgIsBorewicz(identyfication.text)) return;
    transmitter_addr new_transmitter;
    parse_identyfication(identyfication, new_transmitter);
    auto tr = transmitters.find(new_transmitter);
    if(tr != transmitters.end()) {
        time(&tr->last_reported);
    }
    else {
        transmitters.insert(new_transmitter);
    }
}

void receive_senders_identyfication(int recv_sockfd, transmitters_set transmitters) {
    static list<recv_msg> recv_identyfications;
    receive_pending_messages(recv_sockfd, recv_identyfications);
    for (const recv_msg& id : recv_identyfications)
        update_sender_identyfication(id, transmitters);
}



void clear_not_reported_transmitters(transmitters_set& transmitters) {
    time_t current_time = time(nullptr);
    for (const transmitter_addr& tr : transmitters) {
        if(difftime(tr.last_reported, current_time) > 20) {
            transmitters.erase(tr);
        }
    }
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