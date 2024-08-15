Simple console Server Client for learning purpose.

On G++ both ClientSide and ServerSide starts with additional -lws2_32 command.

Has two options - Transmitting a message to the Server. Or Calculation two Integers at server, that are sent from a Client to a Server, and sending a result(sum) back to a Client.

You can choose a cumstom port by the second argv paramether (argv[2]). In other case it works on a default port 27015.

Server Connection is always active.
Client Connection closes after each interaction. 

