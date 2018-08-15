cmake_minimum_required(VERSION 3.10)
project(zadanie2)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -O2 -pthread")
set(LIBRARY_OUTPUT_PATH ${lboost_filesystem}/lib)
set(CMAKE_CXX_STANDARD 17)

add_executable(zadanie2 sikradio_sender.cpp sikradio_receiver.cpp common/common.cpp common/common.hpp common/datagram_packing.hpp sikradio_sender/audio_transmission.cpp sikradio_sender/audio_transmission.hpp sikradio_sender/sikradio_listening.cpp sikradio_sender/sikradio_listening.hpp sikradio_receiver/receive_managment.cpp sikradio_receiver/receive_managment.hpp sikradio_receiver/receive_mcast_listening.cpp sikradio_receiver/receive_mcast_listening.hpp)

