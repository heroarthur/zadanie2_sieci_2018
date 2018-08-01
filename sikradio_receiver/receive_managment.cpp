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





void radio_receiver::send_lookup() {
    if(true) {//minal rtime
        sendto_msg(lookup_sockfd, lookup_addr, ZERO_SEVEN_COME_IN);
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