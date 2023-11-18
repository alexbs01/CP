cmake_minimum_required(VERSION 3.24)
project(p1_a_becerra_adrian_rego C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(p1_a_becerra_adrian_rego
        array.c
        options.c
        options.h)
