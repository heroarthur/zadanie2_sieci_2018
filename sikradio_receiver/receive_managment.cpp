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




void find_new_transmitter(string priority_transmiter_name, transmitter_addr& tr, transmitters_set& availabile_transmiters) {
    for (auto it = availabile_transmiters.begin(); it != availabile_transmiters.end(); ++it)
    {
        if ((*it).nazwa_stacji == priority_transmiter_name)
            tr = *it;
    }
    if(availabile_transmiters.find(tr) != availabile_transmiters.end())
        return;
    tr = *availabile_transmiters.begin();
}


void restart_audio_player(current_transmitter_session& session, availabile_transmitters& transmitters, uint16_t ctrl_port_u16) {
    pthread_mutex_lock(&session.mutex);
    transmitters.clear_not_reported_transmitters();
    if(transmitters.empty()) return;
    transmitter_addr new_tr;
    if(!transmitters.get_next_transmitter(new_tr)) return;
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


/*
void manage_receive_audio(int rexmit_sockfd, uint16_t rexmit_port, const uint32_t bsize, transmitters_set& availabile_transmiters, const uint32_t& rexmit_time, current_transmitter_session& session) {
    static ROUND_TIMER rexmit_timer(rexmit_time);

    if(!session.SESSION_ESTABLISHED) {
        restart_audio_player(session, availabile_transmiters, rexmit_port);
    }
    if(!session.SESSION_ESTABLISHED) return;
    //receive_pending_packs(session);
    //update_rexmit(session);
    //pobierz ile sie da, ustawiajac session id i numer pierwszej paczki
    //if(rexmit_timer.new_round_start()) {//kolejny rexmit time
    //    send_rexmit(rexmit_sockfd, session);
    //}
    //write_audio_to_stdin(session);
}

*/

/*
void receive_pending_packs(current_transmitter_session& session) {
    if(!session.SESSION_ESTABLISHED) return;
    static const uint32_t RECEIVE_LIMIT = 10;
    static char buff[RECVFROM_BUFF_SIZE];
    bool socket_clear = false;
    recv_msg m;
    ssize_t numbytes;

    uint64_t session_id;
    uint64_t first_byte_num;
    byte_container recv_raw_bytes{};

    struct sockaddr their_addr{};
    socklen_t addr_len;


    static uint64_t BRAKUJACYCH_PAKIETOW = 0;
    static uint64_t ODEBRANYCH_PAKIETOW = 0;
    static uint64_t last_packg_num = 0;


    uint32_t received_in_frame = 0;
    while(!socket_clear && received_in_frame < RECEIVE_LIMIT) {
        if(received_in_frame == RECEIVE_LIMIT) printf("RECEIVE_LIMIT\n");
        m = recv_msg{};
        //memset(buff, 0, sizeof buff);
        if ((numbytes = recvfrom(session.mcast_sockfd, buff, RECVFROM_BUFF_SIZE-1 , 0, nullptr, nullptr)) == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                socket_clear = true;
                continue;
            }
            else {
                perror("recvfrom");
                continue;
            }
        }
        received_in_frame++;
        get_int64_bit_value_(buff, session_id, 0);
        get_int64_bit_value_(buff, first_byte_num, sizeof(uint64_t));
        uint32_t recv_psize = numbytes - 2*sizeof(uint64_t);
        uint64_t packg_meta_number_len = 2*sizeof(uint64_t);


        //DEBUG
        if(first_byte_num - last_packg_num != PSIZE_DEF)
            BRAKUJACYCH_PAKIETOW++;
        last_packg_num = first_byte_num;
        ODEBRANYCH_PAKIETOW++;




        if(!session.FIRST_PACKS_RECEIVED) update_session_first_pack(session_id, first_byte_num, recv_psize, session);

        if(session_id < session.session_id) continue;
        if(session_id > session.session_id) {
            session.SESSION_ESTABLISHED = false;
            return;
        }

        recv_raw_bytes.create_new(buff, packg_meta_number_len, session.psize);
        if(session.next_packg_to_stdin <= first_byte_num)
            session.packs_dict->insert(first_byte_num, packgs{first_byte_num, recv_raw_bytes});
        session.biggest_received_pack = max<uint64_t>(session.biggest_received_pack, first_byte_num);
        printf("received packed %d \n", first_byte_num - session.byte0); //TODO

        while(session.biggest_received_pack > session.last_coherent_waiting_packgs &&
              session.packs_dict->contain(session.last_coherent_waiting_packgs + session.psize)) {
            session.last_coherent_waiting_packgs += session.psize;
        }
    }
}


void send_rexmit(int rexmit_sockfd, current_transmitter_session& session) {
    if(!session.SESSION_ESTABLISHED) return;
    if(session.missing_packages.empty()) return;
    string missing_packgage_msg = join_container_elements<set<string> >(session.missing_packages, ",");
    string louder_please_msg = LOUDER_PLEASE + " " + missing_packgage_msg;
    printf("%s \n", louder_please_msg.c_str());// TODO
    sendto_msg(rexmit_sockfd, session.tr_addr, louder_please_msg.c_str(), louder_please_msg.length(), session.rexmit_port);
}



void update_rexmit(current_transmitter_session& session) {
    if(!session.SESSION_ESTABLISHED) return;
    if(session.last_coherent_waiting_packgs >= session.biggest_received_pack) return;
    uint64_t rexmit_num = session.last_coherent_waiting_packgs;
    while(rexmit_num < session.biggest_received_pack) {
        rexmit_num += session.psize;
        session.missing_packages.insert(to_string(rexmit_num));
    }
}
*/

//BYTE0 + ⌊BSIZE*3/4⌋
/*
void write_audio_to_stdin(current_transmitter_session& session) {
    static char write_buff[WRITE_BUFF_SIZE];
    static packgs p;

    if(!session.SESSION_ESTABLISHED) return;
    //memset(write_buff, 0, WRITE_BUFF_SIZE);
    if(session.byte0 < 0) return;
    if(session.biggest_received_pack - session.byte0 >= session.bsize*(3/4))
        return;
    while(session.next_packg_to_stdin < session.last_coherent_waiting_packgs) {
        uint64_t next_to_sent = session.next_packg_to_stdin + session.psize;
        if(session.packs_dict->contain(next_to_sent)) {
            p = session.packs_dict->get(next_to_sent);
            p.bytes.write_to_array(write_buff, 0, session.psize);
            printf("%ll \n", session.next_packg_to_stdin); //TODO
        }
        else {
            session.SESSION_ESTABLISHED = false;
        }
    }
}
*/


void init_transmitter_session(current_transmitter_session& session, const transmitter_addr& tr, uint16_t ctrl_port) {
    //session.rexmit_port = ctrl_port;
    session.SESSION_ESTABLISHED = true;
    session.ctrl_port_u16 = ctrl_port;
    //session.tr_addr = tr.direct_rexmit_con;
    session.FIRST_PACKS_RECEIVED = false;
    ///session.biggest_received_pack = 0;
    //session.next_packg_to_stdin = 0;
    session.byte0 = 0;
    session.session_id = 0;
    session.mcast_addr = tr.mcast_addr;
    session.data_port = tr.data_port;
   // session.audio_buff.clear();
    session.packs_dict->clear();
   // session.missing_packages.clear();
   // session.last_coherent_waiting_packgs = 0;
   // session.next_packg_to_stdin = 0;
    session.current_transmitter = tr;
}



bool SESSION_ESTABLISHED;
bool FIRST_PACKS_RECEIVED;
uint64_t byte0;
uint64_t session_id;
uint64_t biggest_received_pack;
uint64_t last_coherent_waiting_packgs;
uint64_t last_writed_to_stdin_packgs;


void update_session_first_pack(uint64_t recv_session_id, uint64_t first_byte_num, uint32_t recv_psize, current_transmitter_session& session) {
    session.FIRST_PACKS_RECEIVED = true;
    session.byte0 = first_byte_num;
    session.last_pack_num = first_byte_num;
    session.cur_pack_num = first_byte_num;
//    session.biggest_received_pack = first_byte_num;
    session.psize = recv_psize;
 //   session.next_packg_to_stdin = first_byte_num;
  //  session.last_coherent_waiting_packgs = first_byte_num;
    session.session_id = recv_session_id;
   // session.packs_dict->set_max_size(session.bsize/recv_psize);
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
