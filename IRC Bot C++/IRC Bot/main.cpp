#include "IrcBot.h"

int main()
{
	//use braces to make it clear the default constructor is being called
    IrcBot ausBot{};
    ausBot.run();
    return 0;
}

//TODO change all char[1024] to std::ostringstream to ensure no buffer overflow breaks the program
