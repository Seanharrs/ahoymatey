using System;
using System.Net.Sockets;
using System.IO;
using System.Text.RegularExpressions;
using System.Threading;

namespace IRC_Bot
{
	public class IrcBot
	{
		private enum NextExecutionStep { DoNothing, Continue, Return };
		
		private static BotDetails bot;
		private static TcpClient irc;
		private static StreamWriter writer;
		private static StreamReader reader;
		
		private static void Main()
		{
			bot = new BotDetails("Aus Bot CSharp v0", "Aus_Bot_CSharp", "AusBotHosting", "AusBotCSharp", "irc.quakenet.org", "#ausbot_test");
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
							case NextExecutionStep.Return: return;
							case NextExecutionStep.Continue: continue;
							case NextExecutionStep.DoNothing: break;
						}
						switch(CheckUserMessages(line))
						{
							case NextExecutionStep.Return: return;
							case NextExecutionStep.Continue: continue;
							case NextExecutionStep.DoNothing: break;
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

		/// <summary>Initialises the connection to the server, the reader, and the writer</summary>
		private static void OpenConnection()
		{
			irc = new TcpClient(bot.Server, bot.Port);
			NetworkStream stream = irc.GetStream();
			reader = new StreamReader(stream);
			writer = new StreamWriter(stream);
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

		/// <summary>Checks for important messages from the server</summary>
		/// <param name="line">A line of text (message) from the server</param>
		/// <returns>Whether to skip to the next iteration or continue current loop</returns>
		private static NextExecutionStep CheckServerMessages(string line)
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
					if(bot.AdminNames.Contains(name))
						GiveOp(name);
					SendData("PRIVMSG", bot.Channel + " :Hello, " + name);
				}
			}
			else return NextExecutionStep.DoNothing;
			
			return NextExecutionStep.Continue;
		}

		/// <summary>Checks for commands given from users in the channel</summary>
		/// <param name="line">A line of text (message) from the server</param>
		/// <returns>Whether to skip to the next iteration or continue current loop</returns>
		private static NextExecutionStep CheckUserMessages(string line)
		{
			if(line.Contains(".botquit")) return NextExecutionStep.Return;

			if(line.Contains(".get"))
			{
				Console.WriteLine(line);
				Match match = Regex.Match(line, @":([^!]*)!.*?:\.get ([\S ]*)");
				string name = match.Groups[1].Value;
				string query = match.Groups[2].Value.Replace(' ', '_');
				string url = "https://en.wikipedia.org/wiki/" + query.ToLower();
				SendData("PRIVMSG", bot.Channel + " :" + name + ": " + url);
			}
			else if(line.Contains("MODE") && !line.Contains("PRIVMSG"))
			{
				Console.WriteLine(line);
				string name = Regex.Match(line, @"-o (\S*)").Groups[1].Value;

				if(bot.AdminNames.Contains(name))
					GiveOp(name);
			}
			else return NextExecutionStep.DoNothing;

			return NextExecutionStep.Continue;
		}

		/// <summary>Makes a user a channel operator if they are supposed to be</summary>
		/// <param name="user">The nickname to compare against valid operators</param>
		private static void GiveOp(string user) => SendData("MODE", bot.Channel + " +o " + bot.AdminNames[bot.AdminNames.IndexOf(user)]);

		/// <summary>Closes the reader, writer, and connection to the server</summary>
		private static void CloseConnection()
		{
			writer.Close();
			reader.Close();
			irc.Close();
		}
	}
}