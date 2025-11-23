#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFLEN 512

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <server_ip> <server_port>\n", argv[0]);
        return 1;
    }
    
    char *server_ip = argv[1];
    int server_port = atoi(argv[2]);

    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in srv_addr;
    memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(server_port);
    inet_pton(AF_INET, server_ip, &srv_addr.sin_addr);

    char buffer[BUFLEN];
    socklen_t srv_len = sizeof(srv_addr);

    printf("Type messages to send to server (type 'q' to quit):\n");
    while (1) {
        printf("Client: ");
        if (!fgets(buffer, BUFLEN, stdin)) {
            break;
        }

        if (buffer[0] == 'q' && (buffer[1] == '\n' || buffer[1] == '\0')) {
            // Send quit signal to server
            sendto(sock_fd, buffer, 1, 0, (struct sockaddr*) &srv_addr, srv_len);
            break;
        }

        // Send message to server
        sendto(sock_fd, buffer, strlen(buffer), 0, (struct sockaddr*) &srv_addr, srv_len);

        // Receive reply from server
        ssize_t recv_len = recvfrom(sock_fd, buffer, BUFLEN - 1, 0, NULL, NULL);
        if (recv_len > 0) {
            buffer[recv_len] = '\0';
            printf("Server: %s\n", buffer);
        }
    }

    close(sock_fd);
    return 0;
}
