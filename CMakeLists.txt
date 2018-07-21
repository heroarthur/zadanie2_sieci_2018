cmake_minimum_required(VERSION 3.10)
project(zadanie2)

set(CMAKE_CXX_STANDARD 14)

add_executable(zadanie2 sikradio_sender.cpp sikradio_receiver.cpp common.cpp common.hpp)

