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
        encoding = "utf-8"

        def encode_message(text):
            return text.encode(encoding)

        def decode_message(text):
            return text.decode(encoding)

        def send_data(command):
            command = encode_message(command + "\n")
            print(command)
            irc.send(command)

        def connect():
            global irc
            irc = socket()
            irc.connect((server, port))
            send_data("NICK %s" % nick_name)
            # let the server register the nick command before sending the user command
            sleep(1)
            send_data("USER %s %s %s :%s" % (user_name, host_name, server, real_name))

        def disconnect():
            global irc
            irc.close()

        def join():
            send_data("JOIN %s" % channel)

        def try_op(name):
            for user in names_to_op:
                if user == name:
                    send_data("MODE %s" % (channel + " +o " + name))

        def check_server_messages(message):
            split_line = message.split(" ")
            if split_line[0] == "PING":
                send_data("PONG %s" % split_line[1])

            elif split_line[1] == "JOIN":
                name = match_regex(r":([^!]*)!", split_line[0]).group(1)
                if not (name is None or name == nick_name or name == user_name):
                    send_data("PRIVMSG %s" % (channel + " :Hello, " + name))
                    try_op(name)

            elif split_line[1] == "MODE":
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
                    send_data("PRIVMSG %s" % (channel + " :" + name + ": " + url))

            return NextExecutionStep.DO_NOTHING

        stream_buffer = ""
        count = 0
        joined = False
        try:
            connect()
            while True:
                # join after 4 messages as per IRC protocol
                if not joined and count >= 5:
                    joined = True
                    join()

                stream_buffer = stream_buffer + decode_message(irc.recv(1024))
                lines = stream_buffer.split("\n")
                print(lines)
                # the last element of the list (empty or un-finished message) becomes the buffer
                stream_buffer = lines.pop()
                print(stream_buffer)
                print(lines)
                for line in lines:
                    if "PING" not in line:
                        count += 1
                    # remove any trailing whitespace from the string (\t, \r, spaces, etc.)
                    line = line.rstrip()
                    result = check_server_messages(line)
                    if result == NextExecutionStep.RETURN:
                        return
                    if result == NextExecutionStep.CONTINUE:
                        continue

                    result = check_user_messages(line)
                    if result == NextExecutionStep.RETURN:
                        return
        except Exception as e:
            disconnect()
            print(str(e))
            sleep(10)
            return self.main()


if __name__ == "__main__":
    bot = IrcBot("Aus Bot Python v0", "AUS_Bot_Python", "AusBotHosting",
                 "AusBotPython", "irc.quakenet.org", "#ausbot_test")
    bot.main()
