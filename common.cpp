//
// Created by karol on 21.07.18.
//
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

uint32_t parse_optarg_to_int(int option, char* optarg) {
    char * t;
    auto check = (int64_t)strtol(optarg, &t, 10);
    if ((*t) != '\0' || t == optarg) {
        printf("passed argument wrong %c %s\n", (char)option, optarg);
        exit(1);
    }
    if(check < 0) {
        printf("argument can't be negative have value %c %s\n", (char)option, optarg);
        exit(1);
    }
    auto v = (uint32_t)check;
    return v;
}