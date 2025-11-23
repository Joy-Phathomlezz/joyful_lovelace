How to USE

On terminal 1 (server host):

text
gcc tcp_server.c -o tcp_server
./tcp_server

On terminal 2 (client host):

text
gcc tcp_client.c -o tcp_client
./tcp_client <server_ip>

(use 127.0.0.1 for localhost, or actual IP if across hosts)