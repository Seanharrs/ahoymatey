#include "IrcBot.h"

int main()
{
	//use braces to make it clear the default constructor is being called
    IrcBot ausBot{};
    ausBot.run();
    return 0;
}
