cmake_minimum_required(VERSION 3.30)
project(tftp C)

set(CMAKE_C_STANDARD 11)

add_executable(tftp main.c
        tftp_client.c
        tftp_client.h)
