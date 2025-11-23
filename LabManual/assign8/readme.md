How to Run (in Mininet or real VMs):

    On server host:
    gcc chat_server.c -o chat_server -lpthread
    ./chat_server <port>

    On each client host:
    gcc chat_client.c -o chat_client -lpthread
    ./chat_client <server_ip> <port> <clientNum>
    (e.g., ./chat_client 10.0.0.5 5555 1 for client 1, ...and so on for 2, 3...)

All messages are broadcast to all connected clients and logged to chatlog.txt on the server.