#include "stdafx.h"

class IrcBot
{
    public:
        IrcBot(const char* real, const char* user, const char* host, const char* nick,
               const char* server, const char* channel, const char* port = "6667");
        const char* getRealName() { return realName; }
        const char* getUserName() { return userName; }
        const char* getNickName() { return nickName; }
        const char* getHostName() { return hostName; }
        const char* getServer() { return serverName; }
        const char* getChannel() { return channelName; }
        const char* getPort() { return port; }

        std::vector<const char*> names_to_op =
        {
            "Seanharrs", "SlayerSean",
            "AusBotPython", "Aus_Bot_Python",
            "AusBotCSharp", "Aus_Bot_CSharp",
            "AusBotFSharp", "Aus_Bot_FSharp"
        };

    private:
        const char* realName;
        const char* userName;
        const char* nickName;
        const char* hostName;
        const char* serverName;
        const char* channelName;
        const char* port;
};

IrcBot::IrcBot(const char* real, const char* user, const char* host, const char* nick,
               const char* server, const char* channel, const char* port)
{
    realName = real;
    userName = user;
    hostName = host;
    nickName = nick;
    serverName = server;
    channelName = channel;
    this->port = port;
}

class Program
{
    public:
        void main();

        enum State { DO_NOTHING, CONTINUE, RETURN };

    private:
        std::vector<std::string> explode(const std::string &str, const char &delimiter1, const char &delimiter2);
        State checkServerMessages(const std::string &line);
        State checkUserMessages(const std::string &line);
        void sendData(const char *command, const char *param);
        void openConnection();
        void disconnect();
        void join();

        struct addrinfo *result = nullptr, *ptr = nullptr, hints;

        SOCKET irc = INVALID_SOCKET;
        IrcBot* bot;
        std::string unusedData;
};

void Program::sendData(const char *command, const char *param)
{
    char message[1024];
    sprintf(message, "%s %s", command, param);
    std::cout << message << std::endl;
    send(irc, message, int(strlen(message)), 0);
}

void Program::openConnection()
{
    WSADATA wsaData;
    int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (res != 0)
    {
	    std::cout << "Startup failed!" << std::endl;
	    throw std::exception();
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    res = getaddrinfo("irc.quakenet.org", bot->getPort(), &hints, &result);
	if(res != 0)
	{
		std::cout << "Get address info failed!" << std::endl;
		throw std::exception();
	}

    ptr = result;
    irc = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
    if (irc == INVALID_SOCKET)
    {
        freeaddrinfo(result);
	    std::cout << "Failed to create socket: " + WSAGetLastError() << std::endl;
        throw std::exception();
    }
    res = connect(irc, ptr->ai_addr, int(ptr->ai_addrlen));
    if (res == SOCKET_ERROR)
    {
        closesocket(irc);
        irc = INVALID_SOCKET;
        freeaddrinfo(result);
	    std::cout << "Socket connection failed!" << std::endl;
        throw std::exception();
    }
    sendData("NICK", bot->getNickName());

    char param[1024] = "";
	sprintf(param, "%s %s %s :%s", bot->getUserName(), bot->getHostName(), bot->getServer(), bot->getRealName());
    sendData("USER", param);
}

void Program::disconnect()
{
    shutdown(irc, SD_SEND);
    closesocket(irc);
    WSACleanup();
}

void Program::join() { sendData("JOIN", bot->getChannel()); }

Program::State Program::checkServerMessages(const std::string &line)
{
    std::smatch match;
    if (regex_search(line, match, std::regex(R"(PING :(\S*))")))
    {
        for (auto x : match) std::cout << x;
        std::cout << "HEY!";
        //sendData("PONG", )
    }
    else if (regex_search(line, match, std::regex(R"(":([^!]*)!")")))
    {
        for (auto x : match) std::cout << x;
        std::cout << "HEY!";
        //sendData("JOIN", )
    }
    else if (regex_search(line, match, std::regex(R"("[+-]o (\S*)[^\S]")"))) //orig: "[+-]o (\S*)"
    {
        for (auto x : match) std::cout << x;
        std::cout << "HEY!";
        //sendData("MODE", )
    }
    else return DO_NOTHING;

    return CONTINUE;
}

Program::State Program::checkUserMessages(const std::string &line)
{
    std::smatch match;
    if (regex_search(line, match, std::regex(R"(".*?\.botquit.*?")"))) disconnect();
    else if (regex_search(line, match, std::regex(R"(":([^!]*)!.*?:.get ([\S ]*)[^\S ]")"))) //orig: ":([^!]*)!.*?:.get ([\S ]*)"
    {
        for (auto x : match) std::cout << x;
        std::cout << "HEY!";
        //sendData("PRIVMSG", )
    }
    else return DO_NOTHING;

    return CONTINUE;
}

std::vector<std::string> Program::explode(const std::string &message, const char &delimiter1, const char &delimiter2)
{
    std::vector<std::string> data;
    for (std::string::const_iterator iterator = message.begin(); iterator != message.end(); ++iterator)
    {
        if ((*iterator == delimiter1 || *iterator == delimiter2) && !unusedData.empty())
        {
            data.push_back(unusedData);
            unusedData.clear();
        }
        else if(*iterator != delimiter1 && *iterator != delimiter2) unusedData += *iterator;
    }
    return data;
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "InfiniteRecursion"
void Program::main()
{
    bot = new IrcBot("Aus Bot CPlusPlus v0", "AUS_Bot_CPlusPlus", "AusBotHosting",
                     "AusBotCPlusPlus", "Irc.quakenet.org", "#ausbot_test");
    try
    {
        openConnection();
        int count = 0;
        int failCount = 0;
        std::vector<char> recvBuffer(1024);
        while (true)
        {
            int res = recv(irc, recvBuffer.data(), (int)recvBuffer.size(), 0);
            if (res > 0)
            {
                std::vector<std::string> lines = explode(recvBuffer.data(), '\r', '\n');

                for (size_t i = 0; i < lines.size(); ++i)
                {
                    std::cout << lines[i] << std::endl;
                    ++count;
                    if (count == 4)
                    {
                        join();
                        continue;
                    }
                    switch (checkServerMessages(lines[i]))
                    {
                        case RETURN: return;
                        case CONTINUE: continue;
                        case DO_NOTHING: break;
                    }
                    switch (checkUserMessages(lines[i]))
                    {
                        case RETURN: return;
                        case CONTINUE: continue;
                        case DO_NOTHING: break;
                    }

                }
            }
            else if (++failCount >= 5)
            {
	            std::cout << "Failed to receive data from server." << std::endl;
	            throw std::exception();
            }
        }
    }
    catch (std::exception &e)
    {
        std::string ex = std::string(e.what());
        std::cout << ex << std::endl;
        disconnect();
        std::this_thread::sleep_for(std::chrono::seconds(10));
        main();
    }
    catch (...)
    {
        std::cout << "An exception occurred!" << std::endl;
        disconnect();
        std::this_thread::sleep_for(std::chrono::seconds(10));
        main();
    }
}
#pragma clang diagnostic pop

int main()
{
    Program* program = new Program();
    program->main();
    return 0;
}
