Compile:

text
gcc tcp_fruit_server.c -o tcp_fruit_server
gcc tcp_fruit_client.c -o tcp_fruit_client

Run Server:

text
./tcp_fruit_server 5555

Run Client (any number on different hosts):

    text
    ./tcp_fruit_client <server_ip> 5555 apple 2

Main Concepts Used

    TCP sockets: socket(), bind(), listen(), accept(), connect(), read(), write()

    Data parsing and updating (fruit quantity, last sold timestamp)

    Simple unique customer tracking (<IP, port>)

    Regret message if not enough quantity.

    Client sees running list and total unique customer count every transaction.
