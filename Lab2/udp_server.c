#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFFER_SIZE 30

int main(int argc, char *argv[]) {
    int srv_socket;
    char msg[BUFFER_SIZE];
    int str_len;
    socklen_t clnt_addr_sz;
    struct sockaddr_in serv_addr, clnt_addr;

    srv_socket = socket(PF_INET, SOCK_DGRAM, 0);
    if (srv_socket == -1) { perror("socket"); exit(1); }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    if (bind(srv_socket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        perror("bind");
        exit(1);
    }

    while (1) {
        clnt_addr_sz = sizeof(clnt_addr);
        str_len = recvfrom(srv_socket, msg, BUFFER_SIZE, 0,
                           (struct sockaddr*)&clnt_addr, &clnt_addr_sz);

        if (str_len <= 0) continue;

        if ((str_len == 2 && (msg[0]=='q'||msg[0]=='Q'))) break;

        sendto(srv_socket, msg, str_len, 0,
               (struct sockaddr*)&clnt_addr, clnt_addr_sz);
    }

    close(srv_socket);
    return 0;
}
