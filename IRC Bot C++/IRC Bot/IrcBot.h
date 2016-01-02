#pragma once

#include "BotDetails.h"

class IrcBot
{
	public:
		//could use all optional parameters, but overloading seems nicer here (despite longer code)
		IrcBot();
		IrcBot(const char *, const char *, const char *, const char *, const char *, const char *);
		IrcBot(const char *, const char *, const char *, const char *, const char *, const char *, int);

		void run();

		//use enum class instead of enum to ensure enum name must be put before value
		//e.g. NextExecutionStep::CONTINUE instead of CONTINUE
		enum class NextExecutionStep { DO_NOTHING, CONTINUE, RETURN };

	private:
		vector<string> explode(const string &, const char, const char);
		NextExecutionStep checkServerMessages(const string &);
		NextExecutionStep checkUserMessages(const string &);
		void sendData(const char *, const char *);
		void openConnection();
		void tryOp(const char *);
		void join();

		int irc; //the socket
		struct hostent *server;
		struct sockaddr_in serv_addr;
		BotDetails details;
		string unusedData;
};
