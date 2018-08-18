/*
** selectserver.c -- a cheezy multiperson chat server
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


#include "../common/common.hpp"
#include "ui_menu.hpp"



#define PORT "9034"   // port we're listening on



#include <list>


//https://www.csie.ntu.edu.tw/~r92094/c++/VT100.html



using namespace std;

#define MENU_BUFFER_SIZE 2000
#define BUFFER_SIZE   2000
#define INTERACTION_SIZE 10
#define SET_CURSOR_SIZE 10

#define Up "\x1B\x5B\x41"
#define Down "\x1B\x5B\x42"

#define LABEL "------------------------------------------------------------------------\033E"
#define SIK_RADIO "SIK Radio\033E"


#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))






char interaction[INTERACTION_SIZE];
char character_mode[] = "\377\375\042\377\373\001";
char clear_terminal[] = "\033[2J";
char cursor_upper_left_corner[] = "\033[H";
char reset_terminal[] = "\033c";




struct menu {
    uint32_t row;
    uint32_t line;
    uint32_t optionPointer;
    uint32_t menu_fields;
    list<string> nazwy_odbiornikow;
    char menuScreen[BUFFER_SIZE];
};
typedef struct menu MenuState;



MenuState mainMenu;
MenuState *currentMenu = &mainMenu;






const uint64_t UI_BUFF_SIZE = 10000;
void create_ui_menu(char* ui_menu_buff, list<string> nazwy_odbiornikow) {
    strcat(ui_menu_buff, LABEL);
    strcat(ui_menu_buff, SIK_RADIO);
    strcat(ui_menu_buff, LABEL);
    for(const string& nazwa_odbiornika: nazwy_odbiornikow) {
        strcat(ui_menu_buff, nazwa_odbiornika.c_str());
        strcat(ui_menu_buff, "\033E");
    }
    strcat(ui_menu_buff, LABEL);
}





void setTelnetOptions(int msg_sock) {
    char msg[INTERACTION_SIZE];
    memset(msg, 0, INTERACTION_SIZE);
    strcpy(msg, character_mode);
    strcat(msg, reset_terminal);
    int snd_len = strlen(msg);
    if(write(msg_sock, msg, snd_len) == -1) {
        perror("write erro | check client connection\n");
    }

}


uint32_t parse_optarg_to_int_validation(char* optarg) {
    char * cptr;
    uint32_t v = (uint32_t)strtol(optarg, &cptr, 10);
    if ((*cptr) != '\0' || cptr == optarg) {
        cerr << "passed argument wrong " << optarg << endl;
        exit(1);
    }
    return v;
}


uint32_t parse_optarg_to_int(char* optarg) {
    char * t;
    int64_t check;
    check = (int64_t)strtol(optarg, &t, 10);
    if ((*t) != '\0' || t == optarg) {
        cerr << "port param is wrong type " << optarg << endl;
        exit(1);
    }
    if(check < 1 || check > 65535) {
        cerr << "port in wrong range   " << optarg << endl;
        exit(1);
    }
    uint32_t v = (uint32_t)check;
    return v;
}


void setAnsiCursorPos(char* command, int row, int line) {
    memset(command,0,SET_CURSOR_SIZE);
    snprintf(command, SET_CURSOR_SIZE, "\033[%d;%dH" , line, row);
}





int setOptionPointer(char* interaction, int optionPointer,
                     uint32_t menu_fields) {
    if(strcmp(interaction, Down) == 0)
        return MIN((int)menu_fields, optionPointer+1);
    else if(strcmp(interaction, Up) == 0)
        return MAX(0, optionPointer-1);
    return optionPointer;
}


void changeState(char* interaction, MenuState **menu) {
    (*menu)->optionPointer = setOptionPointer(interaction,
            (*menu)->optionPointer, (*menu)->menu_fields);
}



void create_UI_menu_message(char *buff, char *addBuff, MenuState *menu) {
    setAnsiCursorPos(addBuff, menu->line, menu->row);
    memset(buff, 0, MENU_BUFFER_SIZE);
    strcpy(buff, clear_terminal);
    create_ui_menu(buff, menu->nazwy_odbiornikow);
    strcat(buff, menu->menuScreen);
    strcat(buff, cursor_upper_left_corner);
    strcat(buff, addBuff);
}


void resultMenuScreen(char* interaction, MenuState **menu) {
    changeState(interaction, menu);
}



void sendNewMenuToClient(int msg_sock, char* buff_msg) {
    if(write(msg_sock, "\033[2J", 4)== -1) {
        perror("write erro | check client connection\n");
    }

    if(write(msg_sock, "\033[H", 3)== -1) {
        perror("write erro | check client connection\n");
    }
    int snd_len = strlen(buff_msg);
    if(write(msg_sock, buff_msg, snd_len)== -1) {
        perror("write erro | check client connection\n");
    }
}





int interactionIsValid(char* interaction) {
    if (strcmp(interaction, Down) == 0)
        return 1;
    if (strcmp(interaction, Up) == 0)
        return -1;
    return 0;
}


static char buff[MENU_BUFFER_SIZE];
static char add[10];


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void* support_ui_connection(void* thread_data)
{
    struct UI_THREAD_DATA* config = (UI_THREAD_DATA*)thread_data;
    current_transmitter_session* session = config->session;

    session->reported_transmitters.give_transmitters_names(mainMenu.nazwy_odbiornikow,
                                                           mainMenu.row, mainMenu.line);


    fd_set master;    // master file descriptor list
    fd_set read_fds;  // temp file descriptor list for select()
    int fdmax;        // maximum file descriptor number

    int listener;     // listening socket descriptor
    int newfd;        // newly accept()ed socket descriptor
    struct sockaddr_storage remoteaddr; // client address
    socklen_t addrlen;

    int nbytes;

    char remoteIP[INET6_ADDRSTRLEN];

    int yes=1;        // for setsockopt() SO_REUSEADDR, below
    int i, j, rv;

    struct addrinfo hints, *ai, *p;

    FD_ZERO(&master);    // clear the master and temp sets
    FD_ZERO(&read_fds);

    // get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }

    for(p = ai; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) {
            continue;
        }

        // lose the pesky "address already in use" error message
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }

        break;
    }

    // if we got here, it means we didn't get bound
    if (p == NULL) {
        fprintf(stderr, "selectserver: failed to bind\n");
        exit(2);
    }

    freeaddrinfo(ai); // all done with this

    // listen
    if (listen(listener, 10) == -1) {
        perror("listen");
        exit(3);
    }

    // add the listener to the master set
    FD_SET(listener, &master);

    // keep track of the biggest file descriptor
    fdmax = listener; // so far, it's this one

    static struct timeval tv{0,0};
    tv.tv_sec = 0;
    tv.tv_usec = 250000;

    // main loop
    for(;;) {
        read_fds = master; // copy it
        if (select(fdmax+1, &read_fds, nullptr, nullptr, &tv) == -1) {//&tv
            perror("select");
            exit(4);
        }
        tv.tv_usec = 300000;


        if(session->reported_transmitters.TRANSMITTER_NUMBER_CHANGED) {
            // we got some data from a client
            for(j = 0; j <= fdmax; j++) {
                // send to everyone!
                if (FD_ISSET(j, &master)) {
                    // except the listener and ourselves
                    if (j != listener) {
                        //setTelnetOptions(newfd);
                        session->reported_transmitters.give_transmitters_names(mainMenu.nazwy_odbiornikow,
                                                                               mainMenu.row, mainMenu.line);
                        create_UI_menu_message(buff, add, &mainMenu);
                        sendNewMenuToClient(j, buff);
                    }
                }
            }
            session->reported_transmitters.TRANSMITTER_NUMBER_CHANGED = false;
        }

        for(i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) { // we got one!!
                if (i == listener) {
                    // handle new connections
                    addrlen = sizeof remoteaddr;
                    newfd = accept(listener,
                                   (struct sockaddr *)&remoteaddr,
                                   &addrlen);

                    if (newfd == -1) {
                        perror("accept");
                    } else {
                        FD_SET(newfd, &master); // add to master set
                        if (newfd > fdmax) {    // keep track of the max
                            fdmax = newfd;
                        }
                        printf("selectserver: new connection from %s on "
                               "socket %d\n",
                               inet_ntop(remoteaddr.ss_family,
                                         get_in_addr((struct sockaddr*)&remoteaddr),
                                         remoteIP, INET6_ADDRSTRLEN),
                               newfd);
                        setTelnetOptions(newfd);
                        session->reported_transmitters.give_transmitters_names(mainMenu.nazwy_odbiornikow,
                                                                               mainMenu.row, mainMenu.line);
                        create_UI_menu_message(buff, add, &mainMenu);
                        sendNewMenuToClient(newfd, buff);
                    }
                } else {
                    memset(interaction, 0, sizeof(interaction));
                    if ((nbytes = recv(i, interaction, sizeof interaction, 0)) <= 0) {
                        // got error or connection closed by client
                        if (nbytes == 0) {
                            // connection closed
                            printf("selectserver: socket %d hung up\n", i);
                        } else {
                            perror("recv");
                        }
                        close(i); // bye!
                        FD_CLR(i, &master); // remove from master set
                    } else {
                        resultMenuScreen(interaction, &currentMenu);
                        int move = interactionIsValid(interaction);
                        if(move == 0) continue;
                        session->reported_transmitters.change_transmitter(move);
                        session->reported_transmitters.give_transmitters_names(mainMenu.nazwy_odbiornikow,
                                                                               mainMenu.row, mainMenu.line);
                        create_UI_menu_message(buff, add, &mainMenu);
                        session->SESSION_ESTABLISHED = false;

                        // we got some data from a client
                        for(j = 0; j <= fdmax; j++) {
                            // send to everyone!
                            if (FD_ISSET(j, &master)) {
                                if (j != listener) {
                                    sendNewMenuToClient(j, buff);
                                }
                            }
                        }
                    }
                } // END handle data from client
            } // END got new incoming connection
        } // END looping through file descriptors
    } // END for(;;)--and you thought it would never end!
}