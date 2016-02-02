class BotDetails
  def initialize(real, user, host, nick, server, channel, port)
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
  
  def get_realname
    @real
  end
  
  def get_username
    @user
  end
  
  def get_nickname
    @nick
  end
  
  def get_host
    @host
  end
  
  def get_server
    @server
  end
  
  def get_channel
    @channel
  end
  
  def get_port
    @port
  end
  
  def get_admin_names
    @admin_names
  end
end