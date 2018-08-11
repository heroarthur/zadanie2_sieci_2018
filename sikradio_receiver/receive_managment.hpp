#ifndef ZADANIE2_RECEIVE_MANAGMENT_HPP
#define ZADANIE2_RECEIVE_MANAGMENT_HPP


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
#include <set>

#include "../common/common.hpp"


using namespace std;



void set_default_receiver_arguments(string& discover_addr, string& ui_port, string& ctrl_port,
                                    string&data_port, uint32_t& bsize, uint32_t& rtime);


void set_sikradio_receiver_arguments(const int& argc, char **argv,
                                     string& discover_addr, string& ui_port, string& ctrl_port,
                                     uint32_t& bsize, uint32_t& rtime);





/*
     * Rozpoczynając odtwarzanie, odbiornik:
    1. Czyści bufor, w szczególności porzucając dane w nim się znajdujące, a jeszcze nie
    wyprowadzone na standardowe wyjście.
    2. Jeśli potrzeba, wypisuje się z poprzedniego adresu grupowego, a zapisuje się
    na nowy.
    3. Po otrzymaniu pierwszej paczki audio, zapisuje z niej wartość pola session_id oraz
    numer pierwszego odebranego bajtu (nazwijmy go BYTE0; patrz specyfikacja
    protokołu poniżej), oraz rozpoczyna wysyłanie próśb o retransmisję zgodnie z opisem
    poniżej.
    4. Aż do momentu odebrania bajtu o numerze BYTE0 + ⌊BSIZE*3/4⌋ lub większym,
    odbiornik nie przekazuje danych na standardowe wyjście. Gdy jednak to nastąpi,
    przekazuje dane na standardowe wyjście tak szybko, jak tylko standardowe wyjście
    na to pozwala.*/




struct transmitter_addr{//dodan Connection, popraw i uprosc architekture
    Connection_addres direct_rexmit_con;
    string mcast_addr;
    string data_port;
    string nazwa_stacji;
    mutable uint64_t last_reported_sec;
};


struct current_transmitter_session {
    int mcast_sockfd;
    string mcast_addr;
    string data_port;
    uint16_t rexmit_port;
    Connection_addres tr_addr;
    transmitter_addr current_transmitter;

    uint32_t psize;
    uint32_t bsize;
    set<packgs> audio_buff;
    limited_dict<uint64_t, packgs>* packs_dict;
    set<string> missing_packages;

    bool SESSION_ESTABLISHED;
    bool FIRST_PACKS_RECEIVED;
    uint64_t byte0;
    uint64_t session_id;
    uint64_t biggest_received_pack;
    uint64_t last_coherent_waiting_packgs;
    uint64_t next_packg_to_stdin;
};

void update_session_first_pack(uint64_t recv_session_id, uint64_t first_byte_num, uint32_t recv_psize, current_transmitter_session& session);



struct transmitter_comp {
    bool operator() (const transmitter_addr& lhs, const transmitter_addr& rhs) const
    {
        if(lhs.nazwa_stacji == rhs.nazwa_stacji)
            return 0 < memcmp(&lhs.direct_rexmit_con.ai_addr, &rhs.direct_rexmit_con.ai_addr, sizeof(sockaddr));
        else
            return lhs.nazwa_stacji < rhs.nazwa_stacji;
    }
};

typedef std::set<transmitter_addr, transmitter_comp> transmitters_set;


/*
 * 1. Czyści bufor, w szczególności porzucając dane w nim się znajdujące, a jeszcze nie
wyprowadzone na standardowe wyjście.
2. Jeśli potrzeba, wypisuje się z poprzedniego adresu grupowego, a zapisuje się
na nowy.
3. Po otrzymaniu pierwszej paczki audio, zapisuje z niej wartość pola session_id oraz
numer pierwszego odebranego bajtu (nazwijmy go BYTE0; patrz specyfikacja
protokołu poniżej), oraz rozpoczyna wysyłanie próśb o retransmisję zgodnie z opisem
poniżej.
4. Aż do momentu odebrania bajtu o numerze BYTE0 + ⌊BSIZE*3/4⌋ lub większym,
odbiornik nie przekazuje danych na standardowe wyjście. Gdy jednak to nastąpi,
przekazuje dane na standardowe wyjście tak szybko, jak tylko standardowe wyjście
na to pozwala.
 * */

/*
void restart_audio() {
    audio_buff.clear();
    if(&current_transmitter == nullptr)
        assign_to_next_mcast();
    if(&current_transmitter != nullptr) {
        current_transmitter.first_byte_set = false;
        coherent_waiting_packgs = 0;
        biggest_received_pack_num = 0;
        //bind to mcast adres jak napisze ogolna funcke z socketem
    }
};
*/

void init_transmitter_session(current_transmitter_session& session, const transmitter_addr& tr, uint16_t ctrl_port);

void restart_audio_player(current_transmitter_session& session, transmitters_set& availabile_transmiters);




void receive_pending_packs(current_transmitter_session& session);

void send_rexmit(int rexmit_sockfd, current_transmitter_session& session);


void update_rexmit(current_transmitter_session& session);
//BYTE0 + ⌊BSIZE*3/4⌋

void write_audio_to_stdin(current_transmitter_session& session);



void manage_receive_audio(int rexmit_sockfd, uint16_t rexmit_port, const uint32_t bsize, transmitters_set& availabile_transmiters, const uint32_t& rexmit_time);




template<uint64_t repeat_interval>
void broadcast_lookup(int sockfd, Connection_addres broadcast_addr, const char* msg, const uint64_t msg_len) {
    static uint64_t last_send = current_time_sec();
    static uint64_t cur_time_sec = current_time_sec();
    if(last_send + repeat_interval > cur_time_sec){
        sendto_msg(sockfd, broadcast_addr, msg, msg_len);
        last_send = cur_time_sec;
    }
}

void parse_identyfication(const recv_msg& identyfication, transmitter_addr& transmitter);

void update_sender_identyfication(const recv_msg& identyfication, transmitters_set& transmitters);

void receive_senders_identyfication(int recv_sockfd, transmitters_set& transmitters);

void clear_not_reported_transmitters(transmitters_set& transmitters);

/*
void assign_to_next_mcast() {
    if(finded_transmitters.empty()) return;
    auto it = finded_transmitters.begin();
    current_transmitter = *it;
    finded_transmitters.erase(it);
}


void restart_audio() {
    audio_buff.clear();
    if(&current_transmitter == nullptr)
        assign_to_next_mcast();
    if(&current_transmitter != nullptr) {
        current_transmitter.first_byte_set = false;
        coherent_waiting_packgs = 0;
        biggest_received_pack_num = 0;
        //bind to mcast adres jak napisze ogolna funcke z socketem
    }
};
*/


#endif //ZADANIE2_RECEIVE_MANAGMENT_HPP
