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
#include <algorithm>    // std::min

//#include "../common/datagram_packing.hpp"
#include "../common/common.hpp"
#include "receive_managment.hpp"


using namespace std;


void get_int64_bit_value_(const char* datagram, uint64_t& val, int beg) {
    uint8_t a[8];
    mempcpy(a, datagram + beg, sizeof(uint64_t));
    //v = *((uint64_t*) a);
    for(int i = 0; i < sizeof(uint64_t); i++) {
        ((uint8_t*)&val)[i] = ((uint8_t*) a)[i];
    }
    val = be64toh(val);
}





void parse_identyfication(const recv_msg& identyfication, transmitter_addr& transmitter) {
    vector<string> l = split_string_to_container<vector<string> >(identyfication.text, " ");
    transmitter = {};
    transmitter.mcast_addr = l[second];
    transmitter.data_port = l[third];
    transmitter.nazwa_stacji = join_container_elements<std::vector<string>, from_fourth>(l, " ");
    transmitter.direct_rexmit_con = identyfication.sender_addr;
    transmitter.last_reported_sec = current_time_sec();
}


void update_sender_identyfication(const recv_msg& identyfication, transmitters_set& transmitters) {
    if(!msgIsBorewicz(identyfication.text)) return;
    transmitter_addr new_transmitter;
    parse_identyfication(identyfication, new_transmitter);
    transmitters_set::iterator tr = transmitters.find(new_transmitter);
    if(tr != transmitters.end()) {
        (tr)-> last_reported_sec = current_time_sec();
    }
    else {
        transmitters.insert(new_transmitter);
    }
}

void receive_senders_identyfication(int recv_sockfd, transmitters_set& transmitters) {
    static list<recv_msg> recv_identyfications;
    receive_pending_messages(recv_sockfd, recv_identyfications);
    for (const recv_msg& id : recv_identyfications)
        update_sender_identyfication(id, transmitters);
}



void clear_not_reported_transmitters(transmitters_set& transmitters) {
    time_t current_time = time(nullptr);
    for (const transmitter_addr& tr : transmitters) {
        if(last_report_older_than<20>(tr.last_reported_sec)) {
            transmitters.erase(tr);
        }
    }
}






void restart_audio_player(current_transmitter_session& session, uint16_t ctrl_port_u16) {
    session.reported_transmitters.clear_not_reported_transmitters();
    pthread_mutex_lock(&session.mutex);
    if(session.reported_transmitters.empty()) return;
    transmitter_addr new_tr;
    if(!session.reported_transmitters.get_next_transmitter(new_tr)) return;
    init_transmitter_session(session, new_tr, ctrl_port_u16);
    create_socket_binded_to_new_mcast_addr(session.mcast_sockfd, session.mcast_addr.c_str(), session.data_port.c_str());
    FD_ZERO(&session.mcast_fd_set);
    FD_SET(session.mcast_sockfd, &session.mcast_fd_set);
    pthread_mutex_unlock(&session.mutex);
}





void send_rexmit(int rexmit_sockfd, concurrent_uniqe_list<string>& missing_packs, current_transmitter_session& session) {
    static list<string> missings;
    const uint32_t max_rexmit_lenght = 1200;
    missing_packs.ret_uniqe_list(missings);
    missing_packs.clear();
    if(missings.empty()) return;
    string missing_packgage_msg = join_container_elements<list<string> >(missings, ",");
    missing_packgage_msg = missing_packgage_msg.substr(0, min((uint32_t)missing_packgage_msg.length(), max_rexmit_lenght) );
    string louder_please_msg = LOUDER_PLEASE + " " + missing_packgage_msg;
    sendto_msg(rexmit_sockfd, session.current_transmitter.direct_rexmit_con, louder_please_msg.c_str(), louder_please_msg.length(), session.ctrl_port_u16);
    missings.clear();
}




void init_transmitter_session(current_transmitter_session& session, const transmitter_addr& tr, uint16_t ctrl_port) {
    session.SESSION_ESTABLISHED = true;
    session.ctrl_port_u16 = ctrl_port;
    session.FIRST_PACKS_RECEIVED = false;
    session.byte0 = 0;
    session.session_id = 0;
    session.mcast_addr = tr.mcast_addr;
    session.data_port = tr.data_port;
    session.packs_dict->clear();
    session.current_transmitter = tr;
}



void update_session_first_pack(uint64_t recv_session_id, uint64_t first_byte_num, uint32_t recv_psize, current_transmitter_session& session) {
    session.FIRST_PACKS_RECEIVED = true;
    session.byte0 = first_byte_num;
    session.last_pack_num = first_byte_num;
    session.cur_pack_num = first_byte_num;
    session.psize = recv_psize;
    session.session_id = recv_session_id;
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



void set_default_receiver_arguments(string& discover_addr, string& ui_port, string& ctrl_port,
                                    string&data_port, uint32_t& bsize, uint32_t& rtime){
    discover_addr = DISCOVER_ADDR_DEF;
    data_port = to_string(DATA_PORT_DEF);
    ctrl_port = to_string(CTRL_PORT_DEF);
    ui_port = to_string(UI_PORT_DEF);
    bsize = BSIZE_DEF;
    rtime = RTIME_DEF;
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
