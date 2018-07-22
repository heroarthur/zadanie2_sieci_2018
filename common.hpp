#ifndef ZADANIE2_COMMON_HPP
#define ZADANIE2_COMMON_HPP

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

uint32_t parse_optarg_to_number(int option, char *optarg);


template<uint32_t max_size_>
class Input_fifo_queue {
private:
    uint32_t max_size = max_size_;
    string buff;
    string s;
    char tmp[max_size_];

public:
    Input_fifo_queue() {}
    void read_input();
    string pop_part(uint32_t pop_size);
};


template<uint32_t max_size_>
void Input_fifo_queue<max_size_>::read_input() {
    memset(tmp, 0, max_size);
    read(0, tmp, max_size);
    s = string(tmp);
    if(s.length() + buff.length() <= max_size)
        buff += s;
    else {
        buff = buff.substr(0,max_size - s.length()) + s;
    }
}

template<uint32_t max_size_>
string Input_fifo_queue<max_size_>::pop_part(uint32_t pop_size) {
    string s = buff.substr(0, pop_size);
    buff = buff.substr(pop_size, buff.length());
    return s;
}

#endif //ZADANIE2_COMMON_HPP
