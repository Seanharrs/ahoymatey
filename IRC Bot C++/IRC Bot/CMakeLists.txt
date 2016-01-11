cmake_minimum_required(VERSION 3.3)
project(IRC_Bot)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp IrcBot.cpp IrcBot.h BotDetails.h ProjectIncludes.h)
add_executable(IRC_Bot ${SOURCE_FILES})
