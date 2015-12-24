#include "stdafx.h"

#pragma comment(lib, "Ws2_32.lib")

class IrcBot
{
	public:
		IrcBot(char* real, char* user, char* host, char* nick, char* server, char* channel, char* port);
		char* getRealName() { return realName; }
		char* getUserName() { return userName; }
		char* getNickName() { return nickName; }
		char* getHostName() { return hostName; }
		char* getServer() { return serverName; }
		char* getChannel() { return channelName; }
		char* getPort() { return port; }

		std::vector<char*> names_to_op =
		{
			"Seanharrs", "SlayerSean",
			"AusBotPython", "Aus_Bot_Python",
			"AusBotCSharp", "Aus_Bot_CSharp",
			"AusBotFSharp", "Aus_Bot_FSharp"
		};

	private:
		char* realName;
		char* userName;
		char* nickName;
		char* hostName;
		char* serverName;
		char* channelName;
		char* port;
};

IrcBot::IrcBot(char* real, char* user, char* host, char* nick, char* server, char* channel, char* port = "6667")
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
		std::vector<std::string> explode(const std::string& str, const char& delim1, const char& delim2);
		State checkServerMessages(std::string line);
		State checkUserMessages(std::string line);
		void sendData(char* command, char* param);
		void openConnection();
		void disconnect();
		void join();

		struct addrinfo *result = nullptr, *ptr = nullptr, hints;

		SOCKET irc = INVALID_SOCKET;
		IrcBot* bot;
		std::string unusedData;
};

void Program::sendData(char* command, char* param)
{
	char message[1024];
	sprintf_s(message, "%s %s", command, param);
	std::cout << message << std::endl;
	send(irc, message, int(strlen(message)), 0);
}

void Program::openConnection()
{
	WSADATA wsaData;
	int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (res != 0) throw std::exception("Startup failed!");

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	res = getaddrinfo("irc.quakenet.org", bot->getPort(), &hints, &result);
	if (res != 0) throw std::exception("Get address info failed!");

	ptr = result;
	irc = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
	if (irc == INVALID_SOCKET)
	{
		freeaddrinfo(result);
		throw std::exception("Failed to create socket: " + WSAGetLastError());
	}
	res = connect(irc, ptr->ai_addr, int(ptr->ai_addrlen));
	if (res == SOCKET_ERROR)
	{
		closesocket(irc);
		irc = INVALID_SOCKET;
		freeaddrinfo(result);
		throw std::exception("Socket connection failed!");
	}
	sendData("NICK", bot->getNickName());

	char param[1024] = "";
	sprintf_s(param, "%s %s %s :%s", bot->getUserName(), bot->getHostName(), bot->getServer(), bot->getRealName());
	sendData("USER", param);
}

void Program::disconnect()
{
	//shutdown(irc, SD_SEND);
	closesocket(irc);
	WSACleanup();
}

void Program::join() { sendData("JOIN", bot->getChannel()); }

Program::State Program::checkServerMessages(std::string line)
{
	std::smatch match;
	if (regex_search(line, match, std::regex("PING :(\S*)")))
	{
		for (auto x : match) std::cout << x;
		std::cout << "HEY!";
		//sendData("PONG", )
	}
	else if (regex_search(line, match, std::regex(":([^!]*)!")))
	{
		for (auto x : match) std::cout << x;
		std::cout << "HEY!";
		//sendData("JOIN", )
	}
	else if (regex_search(line, match, std::regex("[+-]o (\S*)")))
	{
		for (auto x : match) std::cout << x;
		std::cout << "HEY!";
		//sendData("MODE", )
	}
	else return DO_NOTHING;

	return CONTINUE;
}

Program::State Program::checkUserMessages(std::string line)
{
	std::smatch match;
	if (regex_search(line, match, std::regex(".*?\.botquit.*?"))) disconnect();
	else if (regex_search(line, match, std::regex(":([^!]*)!.*?:.get ([\S ]*)")))
	{
		for (auto x : match) std::cout << x;
		std::cout << "HEY!";
		//sendData("PRIVMSG", )
	}
	else return DO_NOTHING;

	return CONTINUE;
}

std::vector<std::string> Program::explode(const std::string& message, const char& delim1, const char& delim2)
{
	std::vector<std::string> data;
	for (std::string::const_iterator iter = message.begin(); iter != message.end(); ++iter)
	{
		if ((*iter == delim1 || *iter == delim2) && !unusedData.empty())
		{
			data.push_back(unusedData);
			unusedData.clear();
		}
		else if(*iter != delim1 && *iter != delim2) unusedData += *iter;
	}
	return data;
}

void Program::main()
{
	bot = new IrcBot("Aus Bot CPlusPlus v0", "AUS_Bot_CPlusPlus", "AusBotHosting", "AusBotCPlusPlus", "irc.quakenet.org", "#ausbot_test");
	try
	{
		openConnection();
		int count = 0;
		int failCount = 0;
		std::vector<char> recvBuffer(1024);
		while (true)
		{
			int res = recv(irc, recvBuffer.data(), recvBuffer.size(), 0);
			if (res > 0)
			{
				std::vector<std::string> lines = explode(recvBuffer.data(), '\r', '\n');

				for (size_t i = 0; i < lines.size(); ++i)
				{
					std::string line = lines[i];
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
					switch (checkServerMessages(lines[i]))
					{
						case RETURN: return;
						case CONTINUE: continue;
						case DO_NOTHING: break;
					}

				}
			}
			else if (++failCount >= 5) throw std::exception("Failed to recieve data from server.");
		}
	}
	catch (std::exception& e)
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

int main()
{
	Program* prog = new Program();
	prog->main();
	return 0;
}