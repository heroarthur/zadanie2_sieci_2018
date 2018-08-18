//
// Created by karol on 17.08.18.
//

#ifndef ZADANIE2_UI_MENU_HPP
#define ZADANIE2_UI_MENU_HPP


#include "receive_managment.hpp"

struct UI_THREAD_DATA {
    current_transmitter_session* session;
};
void* support_ui_connection(void* thread_data);




#endif //ZADANIE2_UI_MENU_HPP
