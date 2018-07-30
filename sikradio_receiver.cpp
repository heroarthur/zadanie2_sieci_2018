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

#include "common/common.hpp"

#include "sikradio_receiver/receive_managment.hpp"

using namespace std;




int main_ (int argc, char *argv[]) {
    string discover_addr;
    string ui_port;
    string ctrl_port;
    uint32_t bsize;
    uint32_t rtime;

    set_sikradio_receiver_arguments(argc, argv, discover_addr, ui_port, ctrl_port, bsize, rtime);

    radio_receiver r = radio_receiver(discover_addr, ui_port, ctrl_port, bsize, rtime);


    while(true) {
        r.send_lookup();
        r.receive_senders_identyfication();
        r.manage_audio_package();
    }
}
