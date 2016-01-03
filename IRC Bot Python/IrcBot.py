from socket import socket
from re import search as match_regex
from time import sleep


# Replacement for enum
class NextExecutionStep:
    def __init__(self):
        pass

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
    names_to_op = []

    def __init__(self, real, user, host, nick, server_name, channel_name, server_port=6667):
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
        names_to_op = ["Seanharrs", "SlayerSean",
                       "AusBotFSharp", "Aus_Bot_FSharp",
                       "AusBotCSharp", "Aus_Bot_CSharp",
                       "AusBotCPlusPlus", "Aus_Bot_CPlusPlus"]

    def main(self):
        def send_data(command, param):
            print("%s %s\r\n" % (command, param))
            irc.send("%s %s\r\n" % (command, param))

        def connect():
            global irc
            irc = socket()
            irc.connect((server, port))
            send_data("NICK", nick_name)
            args = "%s %s %s :%s" % (user_name, host_name, server, real_name)
            send_data("USER", args)

        def disconnect():
            global irc
            irc.close()

        def join():
            send_data("JOIN", channel)

        def try_op(name):
            for user in names_to_op:
                if user == name:
                    send_data("MODE", channel + " +o " + name)

        def check_server_messages(message):
            split_line = message.split(" ")
            if split_line[0] == "PING":
                print(message)
                send_data("PONG", split_line[1])

            elif split_line[1] == "JOIN":
                print(message)
                name = match_regex(r":([^!]*)!", split_line[0]).group(1)
                if not (name is None or name == nick_name or name == user_name):
                    send_data("PRIVMSG", channel + " :Hello, " + name)
                    try_op(name)

            elif split_line[1] == "MODE":
                print(message)
                name = match_regex(r"([+-]o) (\S*)", message)
                if name is None:
                    return NextExecutionStep.CONTINUE
                command = name.group(1)
                name = name.group(2)

                if command == "-o":
                    try_op(name)

            else:
                return NextExecutionStep.DO_NOTHING

            return NextExecutionStep.CONTINUE

        def check_user_messages(server_message):
            user_message_begin_index = 3
            split_message = server_message.split(" ")

            if len(split_message) >= user_message_begin_index:
                if split_message[user_message_begin_index] == ":.botquit":
                    disconnect()
                    return NextExecutionStep.RETURN

                if split_message[user_message_begin_index] == ":.get":
                    print(server_message)
                    match = match_regex(":([^!]*)!.*?:.get ([\S ]*)", server_message)
                    name = match.group(1)
                    message = match.group(2).replace(" ", "_")
                    url = "https://en.wikipedia.org/wiki/" + message.lower()
                    send_data("PRIVMSG", channel + " :" + name + ": " + url)

            return NextExecutionStep.DO_NOTHING

        stream_buffer = ""
        count = 0
        try:
            connect()
            while True:
                if count == 4:
                    join()

                stream_buffer = stream_buffer + irc.recv(1024)
                lines = stream_buffer.split("\n")
                stream_buffer = lines.pop()
                for line in lines:
                    line = str(line).rstrip()
                    result = check_server_messages(line)
                    if result == NextExecutionStep.RETURN:
                        return
                    if result == NextExecutionStep.CONTINUE:
                        continue

                    result = check_user_messages(line)
                    if result == NextExecutionStep.RETURN:
                        return
                count += 1
        except Exception as e:
            disconnect()
            print(str(e))
            sleep(10)
            return self.main()


if __name__ == "__main__":
    bot = IrcBot("Aus Bot Python v0", "AUS_Bot_Python", "AusBotHosting",
                 "AusBotPython", "irc.quakenet.org", "#ausbot_test")
    bot.main()
