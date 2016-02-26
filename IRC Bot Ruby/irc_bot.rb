require_relative 'bot_details'
require 'socket'

class IrcBot
  def initialize real, user, host, nick, server, channel, port = 6667
    @details = BotDetails.new real, user, host, nick, server, channel, port
  end

  def run
    begin
      connect

      send_data "NICK", @details.nick
      user = "#{@details.user} #{@details.host} " +
             "#{@details.server} :#{@details.real}"
      sleep 1
      send_data "USER", user
      loop do
        count = 0
        until (line = @irc.gets) == nil
          send_data "JOIN", @details.channel if count == 4
          case check_server_messages line
          when :continue then next
          when :quit then return
          end
          return if (check_user_messages line) == :quit
          count += 1
        end
      end
    rescue StandardError => e
      disconnect
      puts "An exception occurred!\n\n#{e.message}"
      puts "Backtrace: #{e.backtrace.inspect}"
      sleep 10
      raise
      run
    ensure disconnect
    end
  end

  private

  def connect
    @irc = TCPSocket.new @details.server, @details.port
  end

  def send_data command, param
    puts "#{command} #{param}"
    @irc.puts "#{command} #{param}"
    @irc.flush
  end

  def check_server_messages line
    if line.include? "PING"
      puts line
      send_data "PONG", line.split(" ")[1]
    elsif line.include? "JOIN"
      puts line
      if (m = line.match(/:([^!]*)!/))
        name = m.captures[0]
        try_op name if @details.admin_names.include? name
        msg = "#{@details.channel} :Hello, #{name}"
        send_data "PRIVMSG", msg unless name == @details.nick
      end
    else return :do_nothing
    end
    :continue
  end

  def check_user_messages line
    return :quit if line.include? ".botquit"

    if line.include? ".get"
      puts line
      m = line.match(/:([^!]*)!.*?\.get ([\S ]*)/)
      name, query = m.captures
      query.sub! " ", "_"
      url = "https://en.wikipedia.org/wiki/" + query.downcase
      msg = "#{@details.channel} :#{name}: #{url}"
      send_data "PRIVMSG", msg
    elsif line.include?("MODE") && !line.include?("PRIVMSG")
      puts line
      send_data "JOIN", @details.channel
      if (m = line.match(/-o (\S*)/))
        name = m.captures[0]
        try_op name if @details.admin_names.include? name
      end
    else return :do_nothing
    end
    return :continue
  end

  def try_op name
    send_data "MODE", "#{@details.channel} +o #{name}"
  end

  def disconnect
    @irc.close
  end
end
