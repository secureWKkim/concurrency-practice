cmake_minimum_required(VERSION 4.2)
project(concurrency_practice C)

set(CMAKE_C_STANDARD 23)

add_executable(concurrency_practice main.c
        good_mutex.c)
