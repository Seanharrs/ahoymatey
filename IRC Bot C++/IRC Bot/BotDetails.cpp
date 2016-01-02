#include "BotDetails.h"

BotDetails::BotDetails(const char *real, const char *user, const char *host, const char *nick,
           const char *server, const char *channel, int port)
{
	realName = real;
	userName = user;
	hostName = host;
	nickName = nick;
	serverName = server;
	channelName = channel;
	serverPort = port;
}

const char * BotDetails::getRealName() { return realName; }

const char* BotDetails::getUserName() { return userName; }

const char* BotDetails::getNickName() { return nickName; }

const char* BotDetails::getHostName() { return hostName; }

const char* BotDetails::getServer() { return serverName; }

const char* BotDetails::getChannel() { return channelName; }

int BotDetails::getPort() { return serverPort; }

vector<const char*> BotDetails::getAdminNames() { return names_to_op; }
