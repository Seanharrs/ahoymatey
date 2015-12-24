using System;
using System.Collections.Generic;
using System.Net.Sockets;
using System.IO;
using System.Text.RegularExpressions;
using System.Threading;

namespace IRC_Bot
{
	/// <summary>Wrapper class containing a bot's name, server, channel, and other important variables</summary>
	public class IrcBot
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

		public List<string> namesToOp { get; set; } = new List<string>
		{
			"Seanharrs", "SlayerSean",
			"AusBotPython", "Aus_Bot_Python",
			"AusBotFSharp", "Aus_Bot_FSharp",
			"AusBotCPlusPlus", "Aus_Bot_CPlusPlus"
		};

		/// <summary>Initialises a new IRC Bot with the parameters provided</summary>
		/// <param name="real">Additional information about the bot</param>
		/// <param name="user">The name used by the esrver to identify the bot</param>
		/// <param name="host">The bot's host name</param>
		/// <param name="nick">The name that appears on the sidebar of the IRC client</param>
		/// <param name="server">The server to connect to</param>
		/// <param name="channel">The channel to connect to</param>
		/// <param name="port">The port the server uses, default 6667</param>
		public IrcBot(string real, string user, string host, string nick, string server, string channel, int port = 6667)
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

	public class Program
	{
		private enum State
		{
			DoNothing,
			Continue,
			Return
		};
		
		private static IrcBot bot;
		private static TcpClient irc;
		private static StreamWriter writer;
		private static StreamReader reader;
		
		private static void Main()
		{
			bot = new IrcBot("Aus Bot CSharp v0", "AUS_Bot_CSharp", "AusBotHosting", "AusBotCSharp", "irc.quakenet.org", "#ausbot_test");
			try
			{
				OpenConnection();

				SendData("NICK", bot.NickName);
				string userDetails = bot.UserName + " " + bot.Host + " " + bot.Server + " :" + bot.RealName;
				SendData("USER", userDetails);
				while(true)
				{
					string line;
					int count = 0;
					while((line = reader.ReadLine()) != null)
					{
						if(count == 4) SendData("JOIN", bot.Channel);
						switch(CheckServerMessages(line))
						{
							case State.Return: return;
							case State.Continue: continue;
							case State.DoNothing: break;
						}
						switch(CheckUserMessages(line))
						{
							case State.Return: return;
							case State.Continue: continue;
							case State.DoNothing: break;
						}
						++count;
					}
				}
			}
			catch(Exception e)
			{
				CloseConnection();
				Console.WriteLine("An exception occurred!\n\n" + e);
				Thread.Sleep(10000);
				Main();
			}
			finally { CloseConnection(); }
		}

		/// <summary>Checks for important messages from the server</summary>
		private static State CheckServerMessages(string line)
		{
			if(line.Contains("PING"))
			{
				Console.WriteLine(line);
				SendData("PONG", line.Split(' ')[1]);
			}
			else if(line.Contains("JOIN"))
			{
				Console.WriteLine(line);
				string name = Regex.Match(line, @":([^!]*)!").Groups[1].Value;
				if(name != bot.NickName)
				{
					TryOp(name);
					SendData("PRIVMSG", bot.Channel + " :Hello, " + name);
				}
			}
			else if(line.Contains("MODE"))
			{
				Console.WriteLine(line);
				string name = Regex.Match(line, @"[+-]o (\S*)").Groups[1].Value;

				if(line.Contains("-o") && bot.namesToOp.Contains(name)) TryOp(name);
				else if(line.Contains("+o") && !bot.namesToOp.Contains(name)) bot.namesToOp.Add(name);
			}
			else return State.DoNothing;
			
			return State.Continue;
		}

		/// <summary>Checks for commands given from users in the channel</summary>
		private static State CheckUserMessages(string line)
		{
			if(line.Contains(".botquit"))
			{
				CloseConnection();
				return State.Return;
			}
			if(line.Contains(".get"))
			{
				Console.WriteLine(line);
				Match match = Regex.Match(line, @":([^!]*)!.*?:.get ([\S ]*)");
				string name = match.Groups[1].Value;
				string query = match.Groups[2].Value.Replace(' ', '_');
				string url = "https://en.wikipedia.org/wiki/" + query.ToLower();
				SendData("PRIVMSG", bot.Channel + " :" + name + ": " + url);
			}
			else return State.DoNothing;

			return State.Continue;
		}

		/// <summary>Makes a user a channel operator if they are supposed to be</summary>
		/// <param name="user">The nickname to compare against valid operators</param>
		private static void TryOp(string user)
		{
			if(user == bot.NickName) return;

			int opIndex = bot.namesToOp.IndexOf(user);
			if(opIndex >= 0) SendData("MODE", bot.Channel + " +o " + bot.namesToOp[opIndex]);
		}

		/// <summary>Initialises the connection to the server, the reader, and the writer</summary>
		private static void OpenConnection()
		{
			irc = new TcpClient(bot.Server, bot.Port);
			NetworkStream stream = irc.GetStream();
			reader = new StreamReader(stream);
			writer = new StreamWriter(stream);
		}

		/// <summary>Closes the reader, writer, and connection to the server</summary>
		private static void CloseConnection()
		{
			writer.Close();
			reader.Close();
			irc.Close();
		}

		/// <summary>Sends a command to the server</summary>
		/// <param name="command">The command being sent</param>
		/// <param name="param">The arguments required for the command</param>
		private static void SendData(string command, string param)
		{
			writer.WriteLine(command + " " + param);
			Console.WriteLine(command + " " + param);
			writer.Flush();
		}
	}
}