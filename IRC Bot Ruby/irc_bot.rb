require_relative 'bot_details'
require 'socket'

class IrcBot
  def initialize(real, user, host, nick, server, channel, port = 6667)
    @details = BotDetails.new(real, user, host, nick, server, channel, port)
  end
  
  def run
    begin
      connect
      
      send_data("NICK", @details.get_nickname)
      user = "#{@details.get_username} #{@details.get_host} " +
             "#{@details.get_server} :#{@details.get_realname}"
      sleep(1)
      send_data("USER", user)
      while true do
        line = ""
        count = 0
        until (line = @irc.gets) == nil
          send_data("JOIN", @details.get_channel) if count == 4
          case check_server_messages(line)
          when :continue then next
          when :quit then return
          end
          return if check_user_messages(line) == :quit
          count += 1
        end
      end
    rescue StandardError => e
      disconnect
      puts "An exception occurred!\n\n#{e.message}"
      puts "Backtrace: #{e.backtrace.inspect}"
      sleep(10)
      raise
      run
    ensure disconnect
    end
  end
  
  private
  
  def connect
    @irc = TCPSocket.new(@details.get_server, @details.get_port)
  end
  
  def send_data(command, param)
    puts "#{command} #{param}"
    @irc.puts("#{command} #{param}")
    @irc.flush
  end
  
  def check_server_messages(line)
    if (line.include? "PING")
      puts line
      send_data("PONG", line.split(" ").at(1))
    elsif (line.include? "JOIN")
      puts line
      if (m = line.match(/:([^!]*)!/))
        name = m.captures[0]
        try_op(name) if (@details.get_admin_names.include? name)
        send_data("PRIVMSG", "#{@details.get_channel} :Hello, #{name}") if !(name == @details.get_nickname)
      end
    else return :do_nothing
    end
    return :continue
  end
  
  def check_user_messages(line)
    return :quit if (line.include? ".botquit")
    
    if (line.include? ".get")
      puts line
      m = line.match(/:([^!]*)!.*?\.get ([\S ]*)/)
      name, query = m.captures
      query.sub!(" ", "_")
      url = "https://en.wikipedia.org/wiki/" + query.downcase
      send_data("PRIVMSG", "#{@details.get_channel} :#{name}: #{url}")
    elsif (line.include? "MODE") && !(line.include? "PRIVMSG")
      puts line
      send_data("JOIN", @details.get_channel)
      if (m = line.match(/-o (\S*)/))
        name = m.captures[0]
        try_op(name) if (@details.get_admin_names.include? name)
      end
    else return :do_nothing
    end
    return :continue
  end
  
  def try_op(name)
    send_data("MODE", "#{@details.get_channel} +o #{name}")
  end
  
  def disconnect
    @irc.close
  end
end
