cmake_minimum_required(VERSION 3.10)
project(zadanie2)

set(LIBRARY_OUTPUT_PATH ${lboost_filesystem}/lib)
set(CMAKE_CXX_STANDARD 14)

add_executable(zadanie2 sikradio_sender.cpp sikradio_receiver.cpp common/common.cpp common/common.hpp common/datagram_packing.hpp sikradio_sender/audio_transmission.cpp sikradio_sender/audio_transmission.hpp sikradio_sender/sikradio_listening.cpp sikradio_sender/sikradio_listening.hpp)

