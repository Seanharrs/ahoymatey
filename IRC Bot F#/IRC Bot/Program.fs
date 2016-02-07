open System
open System.IO
open System.Threading
open System.Net.Sockets
open System.Text.RegularExpressions

type Details(realName: string, userName: string, hostName: string, 
             nickName: string, serverName: string, channelName: string, 
             serverPort: int, namesOfAdmin: string []) =
    let real = realName
    let user = userName
    let nick = nickName
    let host = hostName
    let server = serverName
    let channel = channelName
    let port = serverPort
    let adminNames = namesOfAdmin
    
    member this.Real = real
    member this.User = user
    member this.Nick = nick
    member this.Host = host
    member this.Server = server
    member this.Channel = channel
    member this.Port = port
    member this.AdminNames = adminNames

type IrcBot(botDetails: Details) as self =
    let bot = botDetails
    let irc = new TcpClient(bot.Server, bot.Port)
    let stream = irc.GetStream ()
    let reader = new StreamReader (stream)
    let writer = new StreamWriter (stream)
    
    let (|Quit|NoQuit|) (message: string) = if message.Contains ".botquit" then Quit else NoQuit

    let (|IsMe|IsAdmin|IsOther|) username =
        match username with
        | name when name = bot.Nick -> IsMe
        | name when self.IsAdmin name -> IsAdmin
        | _ -> IsOther
    
    member this.Real = bot.Real
    member this.User = bot.User
    member this.Nick = bot.Nick
    member this.Host = bot.Host
    member this.Server = bot.Server
    member this.Channel = bot.Channel
    member this.Port = bot.Port
    member this.AdminNames = bot.AdminNames

    member this.ReadLine () = reader.ReadLine ()

    member this.Disconnect () =
        reader.Close ()
        writer.Close ()
        irc.Close ()

    member this.SendData command args =
        let message = command + " " + args
        Console.WriteLine message
        writer.WriteLine message
        writer.Flush ()
        
    member this.IsAdmin user = bot.AdminNames |> Array.contains user
    member this.MakeAdmin user = bot.Channel + " +o " + user |> self.SendData "MODE"

    member this.WelcomeUser user = bot.Channel + " :Hello, " + user |> self.SendData "PRIVMSG"
    
    member this.GetMatchValue (group: int) (m: Match) = m.Groups.[group].Value
    member this.GetRegexValue text pattern group = Regex.Match(text, pattern) |> self.GetMatchValue group

    member this.ParseMessage (message: string) =
        match message with
        | msg when msg.Contains "PING" 
            -> Console.WriteLine msg
               msg.Split(' ').[1] |> self.SendData "PONG"
               false
        | msg when msg.Contains "No ident response" 
            -> self.SendData "JOIN" bot.Channel
               false
        | msg when msg.Contains "JOIN"
            -> Console.WriteLine msg
               let user = (self.GetRegexValue msg @"(:[^!]*)!" 1).Remove(0, 1)
               match user with
               | IsAdmin -> self.MakeAdmin user
                            self.WelcomeUser user
               | IsOther -> self.WelcomeUser user
               | IsMe -> ()
               false
        | msg when msg.Contains ".get"
            -> Console.WriteLine msg
               let regexmatch = Regex.Match(msg, @":([^!]*)!.*?:\.get ([\S ]*)")
               let name = self.GetMatchValue 1 regexmatch
               let query = (self.GetMatchValue 2 regexmatch).Replace(' ', '_')
               let url = "https://en.wikipedia.org/wiki/" + query.ToLower ()
               bot.Channel + " :" + name + ": " + url |> self.SendData "PRIVMSG"
               false
        | msg when msg.Contains "MODE" && msg.Contains "PRIVMSG" |> not
            -> Console.WriteLine msg
               let name = Regex.Match(msg, @"-o (\S*)").Groups.[1].Value
               if self.IsAdmin name then self.MakeAdmin name
               false
        | Quit -> true
        | _ -> false
    
    member this.DoWork calls =
        if calls = 4 then self.SendData "JOIN" bot.Channel

        let quit = self.ReadLine () |> self.ParseMessage
        if not quit then calls + 1 |> self.DoWork

[<EntryPoint>]
let main argv =
    let bot =
        new IrcBot(
            new Details(
                "Aus Bot FSharp v0",
                "Aus_Bot_FSharp",
                "AusBothosting",
                "AusBotFSharp",
                "irc.quakenet.org",
                "#ausbot_test",
                6667,
                [| "Seanharrs"; "SlayerSean";
                "AusBotPython"; "Aus_Bot_Python";
                "AusBotCSharp"; "Aus_Bot_CSharp";
                "AusBotCPlusPlus"; "Aus_Bot_CPlusPlus";
                "AusBotRuby"; "Aus_Bot_Ruby" |]
            )
        )
        
    bot.SendData "NICK" bot.Nick
    bot.User + " " + bot.Host + " " + bot.Server + " :" + bot.Real |> bot.SendData "USER"

    bot.DoWork 1
    0
