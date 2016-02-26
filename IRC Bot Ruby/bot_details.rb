class BotDetails
  attr_reader :real, :user, :host, :nick, :server, :channel, :port, :admin_names

  def initialize real, user, host, nick, server, channel, port
    @real = real
    @user = user
    @host = host
    @nick = nick
    @server = server
    @channel = channel
    @port = port
    @admin_names = ["Seanharrs", "SlayerSean",
                    "AusBotPython", "Aus_Bot_Python",
                    "AusBotCSharp", "Aus_Bot_CSharp",
                    "AusBotFSharp", "Aus_Bot_FSharp",
                    "AusBotCPlusPlus", "Aus_Bot_CPlusPlus"]
  end
end
