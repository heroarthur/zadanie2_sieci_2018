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


uint32_t parse_optarg_to_int(int option, char* optarg) {
    char * t;
    int64_t check;
    check = (int64_t)strtol(optarg, &t, 10);
    if ((*t) != '\0' || t == optarg) {
        cerr << "passed argument wrong " <<(char)(option) << " " << optarg << endl;
        exit(1);
    }
    if(check < 0) {
        cerr << "passed argument negative value  " <<(char)(option) << " " << optarg << endl;
        exit(1);
    }
    uint32_t v = (uint32_t)check;
    return v;
}

//./zadanie2 -a "123.123.123.123" -P 1234 -n "spitfire nazwa odbiornika" -C 4444  -p 900 -f 300 -R 250
void write_program_parameters (string mcast_addr, string nazwa_odbiornika,
                              uint32_t data_port, uint32_t ctrl_port,
                              uint32_t psize, uint32_t fsize, uint32_t rtime) {
    printf("program arguments:\n\
    mcast_addr: %s\n\
    nazwa_odbiornika: %s\n\
    data_port: %u\n\
    ctrl_port: %u\n\
    psize: %u\n\
    fsize: %u\n\
    rtime: %u\n",
           mcast_addr.c_str(), nazwa_odbiornika.c_str(),
           data_port, ctrl_port, psize, fsize, rtime);
}


int main (int argc, char *argv[]) {
    string mcast_addr;
    string nazwa_odbiornika;
    uint32_t data_port;
    uint32_t ctrl_port;
    uint32_t psize;
    uint32_t fsize;
    uint32_t rtime;

    int c;
    opterr = 0;
    int index;

    while ((c = getopt(argc, argv, "a:d:p:b:n:f:P:C:R:")) != -1)
        switch (c) {
            case 'a':
                mcast_addr = string(optarg);
                printf("value of optarg: %s\n", optarg);
                break;
            case 'P':
                data_port = parse_optarg_to_int('P', optarg);
                break;
            case 'n':
                nazwa_odbiornika = string(optarg);
                break;
            case 'C':
                ctrl_port = parse_optarg_to_int('C', optarg);
                break;
            case 'p':
                psize = parse_optarg_to_int('p', optarg);
                break;
            case 'f':
                fsize = parse_optarg_to_int('f', optarg);
                break;
            case 'R':
                rtime = parse_optarg_to_int('R', optarg);
                break;

            case '?':
                if (optopt == 'c')
                    fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                else if (isprint(optopt))
                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                else
                    fprintf(stderr,
                            "Unknown option character `\\x%x'.\n",
                            optopt);
                return 1;
            default:
                abort();
        }


    for (index = optind; index < argc; index++) {
        cerr << "Non-option argument\n";
        return 1;
    }


    write_program_parameters(mcast_addr, nazwa_odbiornika, data_port, ctrl_port, psize, fsize, rtime);
}
