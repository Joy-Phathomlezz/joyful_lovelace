#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>

int main() {
    short PORT = 6969;
    int serverSocket_fd, clientSocket_fd;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    serverSocket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket_fd < 0) {
        perror("error to create socket");
        exit(1);
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(PORT);

    if (bind(serverSocket_fd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("failed to bind the socket");
        exit(1);
    }

    if (listen(serverSocket_fd, 1) < 0) {
        perror("failed to listen");
        exit(1);
    }

    clientSocket_fd = accept(serverSocket_fd, (struct sockaddr*)&clientAddr, &clientAddrLen);
    if (clientSocket_fd < 0) {
        perror("failed to accept the client socket");
        exit(1);
    }

    // operation
    printf("Type q or Q to quit\n");
    char buffer[1024];
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        ssize_t r = read(clientSocket_fd, buffer, sizeof(buffer) - 1);
        if (r <= 0) break;
        buffer[r] = '\0';
        if (buffer[0] == 'q' || buffer[0] == 'Q') {
            printf("Client requested quit\n");
            break;
        }
        printf("Client: %s", buffer);

        printf("You: ");
        if (!fgets(buffer, sizeof(buffer), stdin)) break;
        if (buffer[0] == 'q' || buffer[0] == 'Q') {
            write(clientSocket_fd, "q\n", 2);
            break;
        }
        write(clientSocket_fd, buffer, strlen(buffer));
    }

    close(clientSocket_fd);
    close(serverSocket_fd);
    return 0;
}
