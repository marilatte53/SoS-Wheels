cmake_minimum_required(VERSION 3.26)
project(Wheels C)

set(CMAKE_C_STANDARD 11)

add_executable(Wheels main.c
        io.c
        io.h)
