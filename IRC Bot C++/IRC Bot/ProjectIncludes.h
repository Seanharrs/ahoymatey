#pragma once

#include <iostream>
#include <stdio.h>
#include <vector>
#include <thread>
#include <regex>
#include <sys/socket.h>
#include <sys/unistd.h>
#include <netdb.h>

//import static members to avoid having to write std:: every time one is used
//only useful when used often or want to make code a bit cleaner (remove for optimisation)
using std::string;
using std::vector;
using std::ostringstream;
using std::cout;
using std::exception;
using std::regex;
