cmake_minimum_required(VERSION 3.30)
project(tftp C)

set(CMAKE_C_STANDARD 11)

add_executable(tftp main.c
        gettftp.c
        gettftp.h
        puttftp.c
        utils.c
        utils.h
        puttftp.h)
