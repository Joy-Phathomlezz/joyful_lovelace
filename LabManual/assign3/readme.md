Server:
    gcc fruit_server.c -o fruit_server
    ./fruit_server 5555

    Clients (from multiple hosts):
    gcc fruit_client.c -o fruit_client
    ./fruit_client <server_ip> 5555 mango 2

Each client will be told:

    Transaction status,

    all customer <IP, port> who ever bought,

    and total unique customers.
