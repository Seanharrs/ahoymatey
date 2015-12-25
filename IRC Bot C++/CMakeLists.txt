cmake_minimum_required(VERSION 3.3)
project(IRC_Bot_C__)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp stdafx.h)
add_executable(IRC_Bot_C__ ${SOURCE_FILES})

#Link WINSOCK
include_directories("/usr/include/w32api")
target_link_libraries(IRC_Bot_C__ ws2_32)
