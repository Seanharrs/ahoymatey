#include "IrcBot.h"

//default values for all BotDetails variables that are assigned in the constructor
#define REAL "Aus Bot CPlusPlus v0"
#define USER "AUS_Bot_CPlusPlus"
#define HOST "AusBotHosting"
#define NICK "AusBotCPlusPlus"
#define SERVER "irc.quakenet.org"
#define CHANNEL "#ausbot_test"
#define PORT 6667

//call the BotDetails constructor using parameters provided (if any) or default parameter values
//in order to initialise the details variable inside the IrcBot class, which must be initialised
IrcBot::IrcBot() : details(REAL, USER, HOST, NICK, SERVER, CHANNEL, PORT)
{ }

IrcBot::IrcBot(string real, string user, string host, string nick, string server, string channel)
		: details(real, user, host, nick, server, channel, PORT)
{ }

IrcBot::IrcBot(string real, string user, string host, string nick, string server, string channel, int port)
		: details(real, user, host, nick, server, channel, port)
{ }

void IrcBot::sendData(const char *command, const char *param)
{
	char message[1024];
	sprintf(message, "%s %s\n", command, param);
	std::cout << message << "\n";
	signal(SIGPIPE, SIG_IGN); //ignore broken sigpipe issues so program won't crash
	send(irc, message, strlen(message), 0);
}

void IrcBot::openConnection()
{
	int family = AF_UNSPEC; //don't specify whether IPv4 or IPv6
	int type = SOCK_STREAM; //stream socket (connection)
	int protocol = IPPROTO_TCP; //tcp client

	irc = socket(family, type, protocol);
	if(irc < 0)
	{
		perror("socket");
		throw std::exception();
	}

	server = gethostbyname(details.getServer().c_str());
	if(server == NULL)
	{
		perror("server");
		throw std::exception();
	}

	//ensure serv_addr is "empty" by making every byte have a value of 0
	bzero((char *)&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET; //AF_UNSPEC causes an error, so make server's family IPv4

	//re-populate serv_addr.sin_addr.s_addr with the value of server.h_addr - the server's address
	bcopy(server->h_addr, (char *)&serv_addr.sin_addr.s_addr, (size_t)server->h_length);

	//set server port using htons to ensure bytes are ordered correctly
	//convert the port into unsigned 16-bit integer (uint16_t) from int
	serv_addr.sin_port = htons((uint16_t)details.getPort());

	//connect the socket 'irc' to the server contained in the serv_addr method
	struct sockaddr *sockServAddr = (struct sockaddr *)&serv_addr;
	int status = connect(irc, sockServAddr, sizeof(serv_addr));
	if(status < 0)
	{
		perror("connect");
		throw std::exception();
	}
}

void IrcBot::tryOp(const string name)
{
	for(string nick : details.getAdminNames())
		if(name == nick)
		{
			char opMessage[1024];
			sprintf(opMessage, "%s %s %s", details.getChannel().c_str(), "+o", name.c_str());
			sendData("MODE", opMessage);
		}
}

void IrcBot::join() { sendData("JOIN", details.getChannel().c_str()); }

IrcBot::NextExecutionStep IrcBot::checkServerMessages(const string &line)
{
	std::smatch match;

	if(regex_search(line, match, std::regex(R"(PING (:\S*))")))
		sendData("PONG", match.str(1).c_str());

	else if(regex_search(line, match, std::regex(R"(:([^!]*)!.*?JOIN)")))
	{
		const string nick = details.getNickName();
		const string name = (string)match.str(1);
		if(name != nick)
		{
			//welcome the user
			char param[1024];
			sprintf(param, "%s :Hello, %s", details.getChannel().c_str(), name.c_str());
			sendData("PRIVMSG", param);
			tryOp(name);
		}
	}
	//TODO MODE command
//	else if(regex_search(line, match, std::regex(R"([+-]o (\S*))")))
//	{
//		for(auto x : match)
//			std::cout << x;
//		std::cout << "HEY!";
//		//sendData("MODE", );
//		//tryOp(/*name*/);
//	}
	else
		return NextExecutionStep::DO_NOTHING;

	return NextExecutionStep::CONTINUE;
}

IrcBot::NextExecutionStep IrcBot::checkUserMessages(const string &line)
{
	std::smatch match;

	if(regex_search(line, match, std::regex(R"(.*?\.botquit.*?)")))
	{
		shutdown(irc, SHUT_RDWR);
		return NextExecutionStep::RETURN;
	}
	else if(regex_search(line, match, std::regex(R"(:([^!]*)!.*?:.get ([\S ]*))")))
	{
		string base = "https://en.wikipedia.org/wiki/";
		string name = (string)match.str(1);
		string topic = (string)match.str(2);
		char url[1024];
		sprintf(url, "%s%s", base.c_str(), topic.c_str());
		char param[1024];
		sprintf(param, "%s :%s: %s", details.getChannel().c_str(), name.c_str(), url);
		sendData("PRIVMSG", param);
	}
	else
		return NextExecutionStep::DO_NOTHING;

	return NextExecutionStep::CONTINUE;
}

vector <string> IrcBot::explode(const string &message, const char delimiter1, const char delimiter2)
{
	vector <string> data;
	for(string::const_iterator iterator = message.begin(); iterator != message.end(); ++iterator)
		if(*iterator == delimiter1 || *iterator == delimiter2 && !unusedData.empty())
		{
			data.push_back(unusedData);
			unusedData.clear();
		}
		else if(*iterator != delimiter1 && *iterator != delimiter2)
			unusedData += *iterator;

	return data;
}

IrcBot::NextExecutionStep IrcBot::processData(const vector<char> &buf, int &count)
{
	vector<string> lines = explode(buf.data(), '\r', '\n');
	NextExecutionStep next;
	for(string line : lines)
	{
		//each line is a server message, program should join after 4th message
		++count;
		next = checkServerMessages(line);
		if(next == NextExecutionStep::RETURN)
			return next;
		if(next == NextExecutionStep::CONTINUE)
			continue;

		next = checkUserMessages(line);
		if(next == NextExecutionStep::RETURN)
			return next;
	}
	return NextExecutionStep::DO_NOTHING;
}

void IrcBot::sendConnectionDetails()
{
	//send nickname to server
	sendData("NICK", details.getNickName().c_str());
	//give the server time to register the "NICK" command
	std::this_thread::sleep_for(std::chrono::seconds(1));
	char param[1024];
	//send username, hostname, and realname to server
	sprintf(param, "%s %s %s :%s", details.getUserName().c_str(), details.getHostName().c_str(),
	        details.getServer().c_str(), details.getRealName().c_str());
	sendData("USER", param);
}

IrcBot::NextExecutionStep IrcBot::readFromServer(vector<char> &buffer, int &messageCount, int &failCount)
{
	//recieve data from server, put data into recvBuffer, no flags (hence 0 argument)
	int res = recv(irc, buffer.data(), buffer.size(), 0);
	std::cout << buffer.data();
	switch(res)
	{
		//recv returned error
		case -1:
			//print the error every time (in case it's different)
			perror("Failed to receive data from server.");
			//terminate program if recv fails 5 times in a row
			if(++failCount >= 5)
				throw std::exception();
			break;
			//connection was closed
		case 0:
			std::cout << "Connection closed gracefully (but unexpectedly)!" << "\n";
			throw new std::exception();
			//successfully recieved data from server
		default:
			//reset failCount because data was successfully received
			failCount = 0;
			if(processData(buffer, messageCount) == NextExecutionStep::RETURN)
				return NextExecutionStep::RETURN;
			break;
	}
	return NextExecutionStep::DO_NOTHING;
}

//Ignore "endless loop" warning
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
void IrcBot::run()
{
	int messageCount = 0, failCount = 0;
	bool joined = false;
	try
	{
		openConnection();
		sendConnectionDetails();
		while(true)
		{
			vector<char> recvBuffer(1024);
			//join after 4th message as per IRC standards (ignore PING messages though)
			if(messageCount >= 5 && !joined)
			{
				joined = true;
				join();
			}
			NextExecutionStep next = readFromServer(recvBuffer, messageCount, failCount);
			if(next == NextExecutionStep::RETURN)
				return;
		}
	}
	catch(...)
	{
		std::cout << "An unexpected exception occurred!" << "\n";
		//disable both read and write permissions on the socket
		shutdown(irc, SHUT_RDWR);
		//close the socket
		close(irc);
		std::this_thread::sleep_for(std::chrono::seconds(10));
		run();
	}
}
#pragma clang diagnostic pop
