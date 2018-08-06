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
    Connection con_for_rexmit;
    string mcast_addr;
    string data_port;
    string nazwa_stacji;
    uint64_t first_byte;
    bool first_byte_set;
};








class radio_receiver {
private:
    struct transmitter_comp {
        bool operator() (const transmitter_addr& lhs, const transmitter_addr& rhs) const
        {return 0 < memcmp(&lhs.con_for_rexmit, &rhs.con_for_rexmit, sizeof(struct Connection));}//dodaj porownanie potem
        //by uzyskac sortowanie po nazwach
    };

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

    int lookup_sockfd;
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


void radio_receiver::receive_senders_identyfication() {
    static char buff[10000];
    struct sockaddr_storage their_addr;
    socklen_t addr_len;
    string recv_msg;
    recv_msg_from(recv_msg, lookup_sockfd, (struct sockaddr *)&their_addr, addr_len);
    //void recv_msg_from(string& recv_msg, const Connection& connection);
    if (recvfrom(lookup_sockfd, buff, sizeof(buff), 0, (struct sockaddr *)&their_addr, &addr_len)) {
        perror("recvfrom");
        exit(1);
    }
    string msg = string(buff);
    if(!msgIsBorewicz(msg)) return;
    vector<string> l = split_string_to_container(msg, " ");
    transmitter_addr new_transmitter;
    new_transmitter.mcast_addr = l[second];
    new_transmitter.data_port = l[third];
    new_transmitter.nazwa_stacji = join_container_elements<std::vector, from_fourth>(l, " ");
    new_transmitter.sender_addr_info = their_addr;
    new_transmitter.addr_len = addr_len;
    if(finded_transmitters.find(new_transmitter) != finded_transmitters.end()) {
        finded_transmitters.insert(new_transmitter);
    }
};


#endif //ZADANIE2_RECEIVE_MANAGMENT_HPP
