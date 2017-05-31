cmake_minimum_required(VERSION 3.7)
project(Words_counter)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp measuring_time.h)

add_executable(Words_counter ${SOURCE_FILES})

find_package(Threads REQUIRED)
target_link_libraries(Words_counter Threads::Threads)