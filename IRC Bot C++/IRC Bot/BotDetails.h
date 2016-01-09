#pragma once

#include "ProjectIncludes.h"

class BotDetails
{
	public:
		BotDetails(string real, string user, string host, string nick,
		           string server, string channel, int port);

		string getRealName();
		string getUserName();
		string getNickName();
		string getHostName();
		string getServer();
		string getChannel();
		int getPort();
		vector<string> getAdminNames();

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
