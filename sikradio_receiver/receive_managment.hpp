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
#include <atomic>


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






/*
 * struct current_transmitter_session {
    int mcast_sockfd;
    string mcast_addr;
    string data_port;
    uint16_t rexmit_port;
    Connection_addres tr_addr;
    transmitter_addr current_transmitter;

    uint32_t psize;
    uint32_t bsize;
    set<packgs> audio_buff;
    limited_concurrent_map<uint64_t, byte_container>* packs_dict;
    set<string> missing_packages;

    bool SESSION_ESTABLISHED;
    bool FIRST_PACKS_RECEIVED;
    uint64_t byte0;
    uint64_t session_id;
    uint64_t biggest_received_pack;
    uint64_t last_coherent_waiting_packgs;
    uint64_t next_packg_to_stdin;
};
 * */

struct current_transmitter_session {
    //std::atomic<bool> SESSION_ESTABLISHED(false);
    bool SESSION_ESTABLISHED;
    bool FIRST_PACKS_RECEIVED;
    pthread_mutex_t	mutex;


    int mcast_sockfd;
    fd_set mcast_fd_set;

    string mcast_addr;
    string data_port;
    uint16_t ctrl_port_u16;

    uint64_t last_pack_num;
    uint64_t cur_pack_num;

    transmitter_addr current_transmitter;
    //limited_concurrent_map<uint64_t, byte_container>* packs_dict;
    limited_dict<uint64_t, byte_container>* packs_dict;

    uint64_t byte0;
    uint64_t session_id;
    uint32_t psize;
};

void update_session_first_pack(uint64_t recv_session_id, uint64_t first_byte_num, uint32_t recv_psize, current_transmitter_session& session);







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

void restart_audio_player(current_transmitter_session& session, availabile_transmitters& transmitters, uint16_t ctrl_port_u16);




//void receive_pending_packs(current_transmitter_session& session);

//void send_rexmit(int rexmit_sockfd, current_transmitter_session& session);
void send_rexmit(int rexmit_sockfd, concurrent_uniqe_list<string>& missing_packs, current_transmitter_session& session);



void update_rexmit(current_transmitter_session& session);
//BYTE0 + ⌊BSIZE*3/4⌋

//void write_audio_to_stdin(current_transmitter_session& session);



void manage_receive_audio(int rexmit_sockfd, uint16_t rexmit_port, const uint32_t bsize, transmitters_set& availabile_transmiters, const uint32_t& rexmit_time, current_transmitter_session& session);





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
