#pragma once

#include "ProjectIncludes.h"

class BotDetails
{
	public:
		BotDetails(const char *real, const char *user, const char *host, const char *nick,
		           const char *server, const char *channel, int port);

		const char* getRealName();
		const char* getUserName();
		const char* getNickName();
		const char* getHostName();
		const char* getServer();
		const char* getChannel();
		int getPort();
		vector<const char*> getAdminNames();

	private:
		const char *realName;
		const char *userName;
		const char *nickName;
		const char *hostName;
		const char *serverName;
		const char *channelName;
		int serverPort;

		vector<const char *> names_to_op =
		{
				"Seanharrs", "SlayerSean",
				"AusBotPython", "Aus_Bot_Python",
				"AusBotCSharp", "Aus_Bot_CSharp",
				"AusBotFSharp", "Aus_Bot_FSharp"
		};
};
