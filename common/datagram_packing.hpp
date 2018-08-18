#ifndef ZADANIE2_DATAGRAM_PACKING_HPP
#define ZADANIE2_DATAGRAM_PACKING_HPP


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

using namespace std;



template <typename num>
void memcpy_number(num n, char *datagram, uint32_t begin_pos) {
    uint64_t net_order = htobe64(n);
    memcpy(datagram+begin_pos, ((uint8_t*)&net_order), sizeof(num));
}




#endif //ZADANIE2_DATAGRAM_PACKING_HPP
