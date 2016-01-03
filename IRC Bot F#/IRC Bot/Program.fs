open System
open System.IO
open System.Threading
open System.Net.Sockets
open System.Text.RegularExpressions

let Real = "Aus Bot FSharp v0"
let User = "AUS_Bot_FSharp"
let Host = "AusBotHosting"
let Nick = "AusBotFSharp"
let Server = "irc.quakenet.org"
let Channel = "#ausbot_test"
let Port = 6667

let mutable namesToOp = [| "Seanharrs"; "SlayerSean";
                           "AusBotPython"; "Aus_Bot_Python";
                           "AusBotCSharp"; "Aus_Bot_CSharp";
                           "AusBotCPlusPlus"; "Aus_Bot_CPlusPlus" |]

let currentOp user = Array.exists ((=) user) namesToOp

let connect = 
    let irc = new TcpClient(Server, Port)
    let stream = irc.GetStream()
    let reader = new StreamReader(stream)
    let writer = new StreamWriter(stream)
    irc, reader, writer

let sendmessage (writer: StreamWriter) command parameters = 
    writer.WriteLine(command + " " + parameters)
    Console.WriteLine(command + " " + parameters)
    writer.Flush()

let tryop writer user = sendmessage writer "MODE" (Channel + " +o " + user)

let welcome writer user = sendmessage writer "PRIVMSG" (Channel + " :Hello, " + user)

let disconnect (irc: TcpClient) (reader: StreamReader) (writer: StreamWriter) =
    reader.Close()
    writer.Close()
    irc.Close()
    ()

let checkservercommands (writer: StreamWriter) (line: string) =
    match line with
    | l when l.Contains("PING") 
        -> Console.WriteLine(line)
           sendmessage writer "PONG" (l.Split([|' '|]).[1])
           true
    | l when l.Contains("No ident response")
        -> sendmessage writer "JOIN" Channel
           true
    | l when l.Contains("JOIN") 
        -> Console.WriteLine(line)
           let name = Regex.Match(line, @"(:[^!]*)!").Groups.[1].Value.Remove(0, 1)
           if name <> Nick then tryop writer name
                                welcome writer name
           true
    | l when l.Contains("MODE") 
        -> Console.WriteLine(line)
           let name = Regex.Match(line, @"[+-]o (\S*)").Groups.[1].Value
           if(currentOp name && l.Contains("-o")) then tryop writer name
           else if(not (currentOp name) && l.Contains("+o")) then namesToOp <- Array.append namesToOp [|name|] 
           true
    | _ -> false

let checkusercommands (writer: StreamWriter) (line: string) =
    match line with
    | l when l.Contains(".get") 
        -> Console.WriteLine(line)
           let regexmatch = Regex.Match(line, @":([^!]*)!.*?:.get ([\S ]*)")
           let name = regexmatch.Groups.[1].Value
           let query = regexmatch.Groups.[2].Value.Replace(' ', '_')
           let url = "https://en.wikipedia.org/wiki/" + query.ToLower()
           sendmessage writer "PRIVMSG" (Channel + " :" + name + ": " + url)
    | _ -> ()

let (|Quit|NoQuit|) (line: string) = if line.Contains(".botquit") then Quit else NoQuit

[<EntryPoint>]
let rec main argv =     
    try
        let irc, reader, writer = connect

        sendmessage writer "NICK" Nick
        let param = User + " " + Host + " " + Server + " :" + Real
        sendmessage writer "USER" param

        let mutable quitcommand = false
        while not quitcommand do
            let mutable line = ""
            let mutable count = 0
            while line <> null && not quitcommand do
                line <- reader.ReadLine()
                if(count = 4) then sendmessage writer "JOIN" Channel
                match line with
                | Quit -> quitcommand <- true
                | NoQuit
                    -> let foundcommand = checkservercommands writer line
                       if(not foundcommand) then checkusercommands writer line
                count <- count + 1

    with
    | _ -> Console.WriteLine("An exception occurred!")
           Thread.Sleep(10000)
           main argv |> ignore
    0