cmake_minimum_required(VERSION 3.10)
project(zadanie2)

set(LIBRARY_OUTPUT_PATH ${lboost_filesystem}/lib)
set(CMAKE_CXX_STANDARD 14)

add_executable(zadanie2 sikradio_sender.cpp sikradio_receiver.cpp common.cpp common.hpp datagram_packing.cpp datagram_packing.hpp)

