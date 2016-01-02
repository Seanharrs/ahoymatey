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

IrcBot::IrcBot(const char *real, const char *user, const char *host, const char *nick,
               const char *server, const char *channel
              ) :
		details(real, user, host, nick, server, channel, PORT)
{ }

IrcBot::IrcBot(const char *real, const char *user, const char *host, const char *nick,
               const char *server, const char *channel, int port
              ) :
		details(real, user, host, nick, server, channel, port)
{ }

void IrcBot::sendData(const char *command, const char *param)
{
	char message[1024];
	sprintf(message, "%s %s", command, param);
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
		//display the error in plain text (rather than error number) to console prefixed by "socket: "
		//then throw an exception to ensure execution does not continue past this point
		perror("socket");
		throw std::exception();
	}

	server = gethostbyname(details.getServer());
	if(server == NULL)
	{
		//display the error in plain text (rather than error number) to console prefixed by "server: "
		//then throw an exception to ensure execution does not continue past this point
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
		//display the error in plain text (rather than error number) to console prefixed by "connect: "
		//then throw an exception to ensure execution does not continue past this point
		perror("connect");
		throw std::exception();
	}

	sendData("NICK", details.getNickName());
	std::this_thread::sleep_for(std::chrono::seconds(1));
	char param[1024];
	sprintf(param, "%s %s %s :%s", details.getUserName(), details.getHostName(), details.getServer(),
	        details.getRealName());
	sendData("USER", param);
}

void IrcBot::tryOp(const char *name)
{
	for(const char *nick : details.getAdminNames())
		if(name == nick)
		{
			char opMessage[1024];
			sprintf(opMessage, "%s %s %s", details.getChannel(), "+o", name);
			sendData("MODE", opMessage);
		}
}

void IrcBot::join() { sendData("JOIN", details.getChannel()); }

IrcBot::NextExecutionStep IrcBot::checkServerMessages(const string &line)
{
	std::smatch match;
	const char *ping = R"(PING :(\S*))";
	const char *join = R"(:([^!]*)!)";
	const char *mode = R"([+-]o (\S*))";

	if(regex_search(line, match, std::regex(ping)))
	{
		for(auto x : match)
			std::cout << x;
		std::cout << "HEY!";
		//sendData("PONG", );
	}
	else if(regex_search(line, match, std::regex(join)))
	{
		for(auto x : match)
			std::cout << x;
		std::cout << "HEY!";
		//sendData("JOIN", );
		//tryOp(/*name*/);
	}
	else if(regex_search(line, match, std::regex(mode)))
	{
		for(auto x : match)
			std::cout << x;
		std::cout << "HEY!";
		//sendData("MODE", );
		//tryOp(/*name*/);
	}
	else
		return NextExecutionStep::DO_NOTHING;

	return NextExecutionStep::CONTINUE;
}

IrcBot::NextExecutionStep IrcBot::checkUserMessages(const string &line)
{
	std::smatch match;
	const char *quit = R"(.*?\.botquit.*?)";
	const char *google_search = R"(:([^!]*)!.*?:.get ([\S ]*))";

	if(regex_search(line, match, std::regex(quit)))
	{
		shutdown(irc, SHUT_RDWR);
		return NextExecutionStep::RETURN;
	}

	else if(regex_search(line, match, std::regex(google_search)))
	{
		for(auto x : match)
			std::cout << x;
		std::cout << "HEY!";
		//sendData("PRIVMSG", )
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

	for(string line : data)
		std::cout << line.data();
	return data;
}

void IrcBot::run()
{
	int count = 0;
	int failCount = 0;
	vector<char> recvBuffer(1024);

	try
	{
		openConnection();
		while(true)
		{
			//std::cout << "Starting" << "\n";
			//join after 4th iteration as per IRC standards
			if(count == 4)
				join();

			//recieve data from server, put data into recvBuffer, no flags (hence 0 argument)
			int res = recv(irc, recvBuffer.data(), recvBuffer.size(), 0);
			//std::cout << recvBuffer.size() << " - size\n";
			//std::cout << recvBuffer.data() << " - data\n";
			if(res > 0)
			{
				vector <string> lines = explode(recvBuffer.data(), '\r', '\n');

				for(string line : lines)
				{
					//std::cout << line << " - line\n";
					switch(checkServerMessages(line))
					{
						case NextExecutionStep::RETURN: return;
						case NextExecutionStep::CONTINUE: continue;
						case NextExecutionStep::DO_NOTHING: break;
					}
					switch(checkUserMessages(line))
					{
						case NextExecutionStep::RETURN: return;
						case NextExecutionStep::CONTINUE: continue;
						case NextExecutionStep::DO_NOTHING: break;
					}
				}
				++count;
			}
			else
			{
				perror("recv fail");
				//increment failCount then check if we have failed 5 times yet
				if(++failCount == 5)
				{
					std::cout << "Failed to receive data from server." << "\n";
					throw std::exception();
				}
			}
			//std::cout << "Ending" << "\n";
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
