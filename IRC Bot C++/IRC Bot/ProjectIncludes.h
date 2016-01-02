#pragma once

#include <iostream>
#include <stdio.h>
#include <vector>
#include <thread>
#include <regex>
#include <sys/socket.h>
#include <sys/unistd.h>
#include <netdb.h>

//import static members for string and vector to avoid having to write std:: every time
//a string or vector type is used
using std::string;
using std::vector;
