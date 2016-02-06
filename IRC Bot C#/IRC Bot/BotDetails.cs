using System.Collections.Generic;

namespace IRC_Bot
{
	/// <summary>Wrapper class containing a bot's name, server, channel, and other important variables</summary>
	public class BotDetails
	{
		/// <summary>Additional information about the bot</summary>
		public string RealName { get; }

		/// <summary>The name used by the server to identify the bot</summary>
		public string UserName { get; }

		/// <summary>The name that appears on the sidebar of the IRC client</summary>
		public string NickName { get; }

		/// <summary>The name of the bot's host</summary>
		public string Host { get; }

		/// <summary>The server to connect to</summary>
		public string Server { get; }

		/// <summary>The channel to connect to</summary>
		public string Channel { get; }

		/// <summary>The port the server uses</summary>
		public int Port { get; }

		/// <summary>The names of people to make OP in the channel</summary>
		public List<string> AdminNames => new List<string>
		{
			"Seanharrs", "SlayerSean",
			"AusBotPython", "Aus_Bot_Python",
			"AusBotFSharp", "Aus_Bot_FSharp",
			"AusBotCPlusPlus", "Aus_Bot_CPlusPlus",
			"AusBotRuby", "Aus_Bot_Ruby"
		};

		/// <summary>Initialises a new IRC Bot with the parameters provided</summary>
		/// <param name="real">Additional information about the bot</param>
		/// <param name="user">The name used by the esrver to identify the bot</param>
		/// <param name="host">The bot's host name</param>
		/// <param name="nick">The name that appears on the sidebar of the IRC client</param>
		/// <param name="server">The server to connect to</param>
		/// <param name="channel">The channel to connect to</param>
		/// <param name="port">The port the server uses, default 6667</param>
		public BotDetails(string real, string user, string host, string nick, string server, string channel, int port = 6667)
		{
			RealName = real;
			UserName = user;
			Host = host;
			NickName = nick;
			Server = server;
			Channel = channel;
			Port = port;
		}
	}
}