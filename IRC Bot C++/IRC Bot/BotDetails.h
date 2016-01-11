#pragma once

#include "ProjectIncludes.h"

class BotDetails
{
	public:
		BotDetails(string real, string user, string host, string nick,
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

		string getRealName() { return realName; }
		string getUserName() { return userName; }
		string getNickName() { return nickName; }
		string getHostName() { return hostName; }
		string getServer() { return serverName; }
		string getChannel() { return channelName; }
		int getPort() { return serverPort; }
		vector<string> getAdminNames() { return names_to_op; }

	private:
		string realName;
		string userName;
		string nickName;
		string hostName;
		string serverName;
		string channelName;
		int serverPort;

		vector<string> names_to_op =
		{
				"Seanharrs", "SlayerSean",
				"AusBotPython", "Aus_Bot_Python",
				"AusBotCSharp", "Aus_Bot_CSharp",
				"AusBotFSharp", "Aus_Bot_FSharp"
		};
};
