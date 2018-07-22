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

class Input_fifo_queue {
private:
    uint32_t max_size;
    char tmp[128000];
    string buff;
    string s;

public:
    Input_fifo_queue() {}
    void read_input();
    string pop_part(uint32_t pop_size);
};



#endif //ZADANIE2_COMMON_HPP
