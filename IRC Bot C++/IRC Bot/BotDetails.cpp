#include "BotDetails.h"

BotDetails::BotDetails(string real, string user, string host, string nick,
           string server, string channel, int port)
{
	realName = real;
	userName = user;
	hostName = host;
	nickName = nick;
	serverName = server;
	channelName = channel;
	serverPort = port;
}

string BotDetails::getRealName() { return realName; }

string BotDetails::getUserName() { return userName; }

string BotDetails::getNickName() { return nickName; }

string BotDetails::getHostName() { return hostName; }

string BotDetails::getServer() { return serverName; }

string BotDetails::getChannel() { return channelName; }

int BotDetails::getPort() { return serverPort; }

vector<string> BotDetails::getAdminNames() { return names_to_op; }
