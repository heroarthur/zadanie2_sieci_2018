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



void create_datagram_socket(int &sockfd, addrinfo &sendto_addr, const string ip_addr, const string port);




struct transmitter_addr{//dodan Connection, popraw i uprosc architekture
    Connection_addres direct_rexmit_con;
    string mcast_addr;
    string data_port;
    string nazwa_stacji;
    time_t last_reported;
};

struct transmitter_comp {
    bool operator() (const transmitter_addr& lhs, const transmitter_addr& rhs) const
    {return 0 < memcmp(&lhs.direct_rexmit_con, &rhs.direct_rexmit_con, sizeof(struct Connection_addr));}//dodaj porownanie potem
    //by uzyskac sortowanie po nazwach
};

typedef std::set<transmitter_addr, transmitter_comp> transmitters_set;



class radio_receiver {
private:


    string discover_addr;
    string ui_port;
    string ctrl_port;
    uint32_t bsize;
    uint32_t rtime;


    uint64_t session_id;
    uint32_t coherent_waiting_packgs;
    uint64_t biggest_received_pack_num;
    transmitter_addr current_transmitter;
    vector<packgs> audio_buff;
    std::set<transmitter_addr, transmitter_comp> finded_transmitters;

    int broadcast_socket;
    Connection_addres broadcast_addr{};

    get_communication_addr(rexmit_lookup_addr, USE_MY_IP, ctrl_port.c_str());
    Connection con_lookup_msg;


    void restart_audio();
    void assign_to_next_mcast();






public:

    radio_receiver(string discover_addr_, string ui_port_, string ctrl_port_, uint32_t bsize_, uint32_t rtime_) :
            discover_addr(discover_addr_), ui_port(ui_port_), ctrl_port(ctrl_port_), bsize(bsize_), rtime(rtime_) {
        create_datagram_socket(con_lookup_msg, ctrl_port, &discover_addr);
        //zrob je nieblokujace

    }
    //zero_seven_come_in
    //zbierz rezultaty (non blocking socket dopoki nie empty) -> uaktualnij liste dostepnych odbiornikow
    //sproboj odebrac (non blocking socket dopuki nie empty)
    //wywal na wyjscie min(2/3*wszystkie dostepne bez brakow, 1)
    //prosba o retransmisje
    void send_lookup();
    void receive_senders_identyfication();
    void manage_audio_package();
};

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


void radio_receiver::assign_to_next_mcast() {
    if(finded_transmitters.empty()) return;
    auto it = finded_transmitters.begin();
    current_transmitter = *it;
    finded_transmitters.erase(it);
}


void radio_receiver::restart_audio() {
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





void broadcast_lookup(int sockfd, Connection_addres broadcast_addr, const char* msg, const uint64_t msg_len);

void parse_identyfication(const recv_msg& identyfication, transmitter_addr& transmitter);

void update_sender_identyfication(const recv_msg& identyfication, transmitters_set& transmitters);

void receive_senders_identyfication(int recv_sockfd, transmitters_set transmitters);


#endif //ZADANIE2_RECEIVE_MANAGMENT_HPP
