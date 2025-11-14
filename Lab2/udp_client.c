#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFFER 30

int main(int argc, char *argv[]) {
    int sock;
    char msg[BUFFER];
    int str_len;
    socklen_t addr_sz;
    struct sockaddr_in srv_addr, from_addr;

    sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock == -1) { perror("socket"); exit(1); }

    srv_addr.sin_family = AF_INET;
    srv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    srv_addr.sin_port = htons(atoi(argv[2]));

    while (1) {
        fputs("Type q or Q (to quit): ", stdout);
        fgets(msg, sizeof(msg), stdin);

        if (!strcmp(msg, "q\n") || !strcmp(msg, "Q\n")) {
            sendto(sock, "q", 1, 0, (struct sockaddr*)&srv_addr, sizeof(srv_addr));
            break;
        }

        sendto(sock, msg, strlen(msg), 0,
               (struct sockaddr*)&srv_addr, sizeof(srv_addr));

        addr_sz = sizeof(from_addr);
        str_len = recvfrom(sock, msg, BUFFER, 0,
                           (struct sockaddr*)&from_addr, &addr_sz);
        msg[str_len] = 0;

        printf("Server: %s", msg);
    }

    close(sock);
    return 0;
}
