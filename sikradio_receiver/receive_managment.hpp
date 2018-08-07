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
                                    uint32_t& bsize, uint32_t& rtime);


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
    time_t last_reported;
};


struct current_transmitter_session {
    uint64_t session_id;
    uint32_t coherent_waiting_packgs;
    uint64_t biggest_received_pack_num;
    uint64_t first_byte;
    Connection_addres transmitter_addr;
    bool session_established;
};

struct transmitter_comp {
    bool operator() (const transmitter_addr& lhs, const transmitter_addr& rhs) const
    {return 0 < memcmp(&lhs.direct_rexmit_con, &rhs.direct_rexmit_con, sizeof(struct Connection_addr));}//dodaj porownanie potem
    //by uzyskac sortowanie po nazwach
};

typedef std::set<transmitter_addr, transmitter_comp> transmitters_set;

void manage_receive_audio() {
    static current_transmitter_session session{};
    static vector<packgs> audio_buff;
    if(!session.session_established) {
        //sproboj zapisac i zrestartowac i nowy mcast
        //assign_to_next_mcast();
        //restart_audio();
    }
    if(!session.session_established) return;
    //pobierz ile sie da, ustawiajac session id i numer pierwszej paczki
    if(//nastepeny rexmit) {
            //wyslij rexmit
    }
    //przekaz dane na wyjscie
}


void broadcast_lookup(int sockfd, Connection_addres broadcast_addr, const char* msg, const uint64_t msg_len);

void parse_identyfication(const recv_msg& identyfication, transmitter_addr& transmitter);

void update_sender_identyfication(const recv_msg& identyfication, transmitters_set& transmitters);

void receive_senders_identyfication(int recv_sockfd, transmitters_set transmitters);

void clear_not_reported_transmitters(transmitters_set transmitters);

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
