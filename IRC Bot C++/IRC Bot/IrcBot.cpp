#include "IrcBot.h"

//default values for all BotDetails variables that are assigned in the constructor
#define REAL "Aus Bot CPlusPlus v0"
#define USER "Aus_Bot_CPlusPlus"
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
	ostringstream message;
	message << command << " " << param << "\n";
	cout << message.str() << "\n";
	signal(SIGPIPE, SIG_IGN); //ignore broken sigpipe issues so program won't crash
	send(irc, message.str().c_str(), strlen(message.str().c_str()), 0);
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
		throw exception();
	}

	server = gethostbyname(details.getServer().c_str());
	if(server == NULL)
	{
		perror("server");
		throw exception();
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
		throw exception();
	}
}

void IrcBot::tryOp(const string name)
{
	for(string nick : details.getAdminNames())
		if(name == nick)
		{
			ostringstream opMessage;
			opMessage << details.getChannel() << " +o " << name;
			sendData("MODE", opMessage.str().c_str());
		}
}

void IrcBot::join() { sendData("JOIN", details.getChannel().c_str()); }

IrcBot::NextExecutionStep IrcBot::checkServerMessages(const string &line)
{
	std::smatch match;
	regex ping = regex(R"(PING (:\S*))");
	regex join = regex(R"(:([^!]*)!.*?JOIN)");

	if(regex_search(line, match, ping))
		sendData("PONG", match.str(1).c_str());

	else if(regex_search(line, match, join))
	{
		string nick = details.getNickName();
		string name = (string)match.str(1);
		if(name != nick)
		{
			//welcome the user
			ostringstream param;
			param << details.getChannel() << " :Hello, " << name;
			sendData("PRIVMSG", param.str().c_str());
			tryOp(name);
		}
	}
	else return NextExecutionStep::DO_NOTHING;

	return NextExecutionStep::CONTINUE;
}

IrcBot::NextExecutionStep IrcBot::checkUserMessages(const string &line)
{
	std::smatch match;
	regex quit = regex(R"(.*?\.botquit.*?)");
	regex search = regex(R"(:([^!]*)!.*?:\.get ([\S ]*))");
	regex mode = regex(R"(.*?MODE #[^ ]* -o (\S*))");

	if(regex_search(line, match, quit))
		return NextExecutionStep::RETURN;

	else if(regex_search(line, match, search))
	{
		string base = "https://en.wikipedia.org/wiki/";
		string name = (string)match.str(1);
		string topic = (string)match.str(2);
		ostringstream url;
		url << base << topic;
		ostringstream param;
		param << details.getChannel() << " :" << name << ": " << url.str();
		sendData("PRIVMSG", param.str().c_str());
	}
	else if(regex_search(line, match, mode))
		tryOp((string)match.str(1));

	else return NextExecutionStep::DO_NOTHING;

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
		if(checkServerMessages(line) == NextExecutionStep::CONTINUE)
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
	sleep(1);
	ostringstream param;
	//send username, hostname, and realname to the server
	param << details.getUserName() << " " << details.getHostName()
		  << " " << details.getServer() << " :" << details.getRealName();
	sendData("USER", param.str().c_str());
}

IrcBot::NextExecutionStep IrcBot::readFromServer(vector<char> &buffer, int &messageCount, int &failCount)
{
	//recieve data from server, put data into recvBuffer, no flags (hence 0 argument)
	int res = recv(irc, buffer.data(), buffer.size(), 0);
	cout << buffer.data();
	switch(res)
	{
		//recv returned error
		case -1:
			//print the error every time (in case it's different)
			perror("Failed to receive data from server.");
			//terminate program if recv fails 5 times in a row
			if(++failCount >= 5)
				throw exception();
			break;
			//connection was closed
		case 0:
			cout << "Connection closed gracefully (but unexpectedly)!" << "\n";
			throw new exception();
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
		cout << "An unexpected exception occurred!" << "\n";
		closeConnection();
		sleep(10);
		run();
	}
}

void IrcBot::closeConnection()
{
	//disable both read and write permissions on the socket
	shutdown(irc, SHUT_RDWR);
	//close the socket
	close(irc);
}
