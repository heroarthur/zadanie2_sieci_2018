#ifndef ZADANIE2_SIKRADIO_LISTENING_HPP
#define ZADANIE2_SIKRADIO_LISTENING_HPP



struct listening_thread_configuration {
    std::string ctrl_port;
    std::string mcast_addr;
    std::string data_port;
    std::string nazwa_stacji;
    concurrent_uniqe_list<string> *ret_list;
};


void *listening_rexmit_lookup(void *thread_data);


#endif //ZADANIE2_SIKRADIO_LISTENING_HPP
