#ifndef ZADANIE2_SIKRADIO_LISTENING_HPP
#define ZADANIE2_SIKRADIO_LISTENING_HPP


struct listening_thread_configuration {
    //int thread_id;
    string ctrl_port;
    string mcast_addr;
    string data_port;
    string nazwa_stacji;
    concurrent_uniqe_list *ret_list;
};


void* listening_rexmit_lookup(void *thread_data);


#endif //ZADANIE2_SIKRADIO_LISTENING_HPP
