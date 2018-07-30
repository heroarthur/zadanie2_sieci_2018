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

#include "../common/common.hpp"


void set_default_receiver_arguments(string& discover_addr, string& ui_port, string& ctrl_port,
                                    uint32_t& bsize, uint32_t& rtime);


void set_sikradio_receiver_arguments(const int& argc, char **argv,
                                     string& discover_addr, string& ui_port, string& ctrl_port,
                                     uint32_t& bsize, uint32_t& rtime);


class radio_receiver {
private:
    string discover_addr;
    string ui_port;
    string ctrl_port;
    uint32_t bsize;
    uint32_t rtime;


public:
    radio_receiver(string discover_addr_, string ui_port_, string ctrl_port_, uint32_t bsize_, uint32_t rtime_) :
            discover_addr(discover_addr_), ui_port(ui_port_), ctrl_port(ctrl_port_), bsize(bsize_), rtime(rtime_) {}


    //zero_seven_come_in
    //zbierz rezultaty (non blocking socket dopoki nie empty) -> uaktualnij liste dostepnych odbiornikow
    //sproboj odebrac (non blocking socket dopuki nie empty)
    //wywal na wyjscie min(2/3*wszystkie dostepne bez brakow, 1)
    //prosba o retransmisje
    void send_lookup();
    void receive_senders_identyfication();
    void manage_audio_package();
};

#endif //ZADANIE2_RECEIVE_MANAGMENT_HPP
