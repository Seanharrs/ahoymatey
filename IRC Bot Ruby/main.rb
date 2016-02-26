require_relative 'irc_bot'

bot = IrcBot.new("Aus Bot Ruby v0", "Aus_Bot_Ruby", "AusBotHosting",
                 "AusBotRuby", "irc.quakenet.org", "#ausbot_test")

bot.run
