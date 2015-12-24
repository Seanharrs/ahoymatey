import socket
import re
from time import sleep

class State:
    DO_NOTHING = 1
    CONTINUE = 2
    RETURN = 3

class IrcBot:
    real_name = ""
    user_name = ""
    host_name = ""
    nick_name = ""
    server = "" 
    channel = "" 
    port = 0
    irc = None
    names_to_op = [ "Seanharrs", "SlayerSean",
                    "AusBotFSharp", "Aus_Bot_FSharp",
                    "AusBotCSharp", "Aus_Bot_CSharp",
                    "Aus_Bot_CPlusPlus", "AusBotCPlusPlus" ]

    def __init__(self, real, user, host, nick, server_name, channel_name, server_port = 6667):
        global real_name
        global user_name
        global host_name
        global nick_name
        global server
        global channel
        global port
        global names_to_op
        real_name = real
        user_name = user
        host_name = host
        nick_name = nick
        server = server_name
        channel = channel_name
        port = server_port

    def main(self):
        def send_data(command, param):
            print "%s %s\r\n" % (command, param)
            irc.send("%s %s\r\n" % (command, param))

        def connect():
            global irc
            irc = socket.socket()
            irc.connect((server, port))
            send_data("NICK", nick_name)
            args = "%s %s %s :%s" % (user_name, host_name, server, real_name)
            send_data("USER", args) 
        
        def disconnect():
            global irc
            irc.close()
    
        def join(): send_data("JOIN", channel)

        def try_op(name):
            index = names_to_op.index(name)
            send_data("MODE", channel + " +o " + names_to_op[index])

        def check_server_messages(line):
            split_line = line.split(" ")
            if(split_line[0] == "PING"):
                print line
                send_data("PONG", split_line[1])

            elif(split_line[1] == "JOIN"):
                print line
                name = re.match(r":([^!]*)!", line)
                if(not (name == None or name == nick_name)):
                    name = name.group(1)
                    send_data("PRIVMSG", channel + " :Hello, " + name)

            elif(split_line[1] == "MODE"):
                print line
                name = re.search(r"([+-]o) (\S*)", line)
                if(name == None): return State.CONTINUE
                command = name.group(1)
                name = name.group(2)

                in_list = False
                for user in names_to_op:
                    if(user == name):
                        in_list = True
                if(command == "-o" and in_list): try_op(name)
                elif(command == "+o" and not in_list): names_to_op.append(name)

            else: return State.DO_NOTHING

            return State.CONTINUE

        def check_user_messages(line):
            split_line = line.split(" ")
            if(split_line[len(split_line)-1] == ":.botquit"):
                close_connection()
                return State.RETURN

            if(split_line[len(split_line)-1] == ":.get"):
                print line
                match = re.match(":([^!]*)!.*?:.get ([\S ]*)", line)
                name = match.group(1)
                message = match.group(2).replace(" ", "_")
                url = "https://en.wikipedia.org/wiki/" + message.lower()
                send_data("PRIVMSG", channel + " :" + name + ": " + url)

            else: return State.DO_NOTHING
            
            return State.CONTINUE
        
        buffer = ""
        try:
            connect()
            count = 0
            while True:
                if(count == 4): join()
                buffer = buffer + irc.recv(1024)
                lines = buffer.split("\n")
                buffer = lines.pop()
                for line in lines:
                    line = str(line)
                    line = line.rstrip()
                    result = check_server_messages(line)
                    if(result == State.RETURN): return
                    if(result == State.CONTINUE): continue
                    
                    result = check_user_messages(line)
                    if(result == State.RETURN): return
                count += 1
        except Exception, e:
            disconnect()
            print str(e)
            sleep(10)
            return self.main()

if __name__ == "__main__":
    bot = IrcBot("Aus Bot Python v0", "AUS_Bot_Python", "AusBotHosting", "AusBotPython", "irc.quakenet.org", "#ausbot_test")
    bot.main()