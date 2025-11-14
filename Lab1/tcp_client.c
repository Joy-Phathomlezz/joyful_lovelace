#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <errno.h>

int main() {
    int PORT = 6969;
    int socket_fd;
    struct sockaddr_in serverAddr;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("socket creation failed");
        exit(1);
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("10.0.0.1");
    serverAddr.sin_port = htons(PORT);

    if (connect(socket_fd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("connection failed");
        exit(1);
    }

    // operation
    printf("Type q or Q to quit\n");
    char buffer[1024];
    while (1) {
        printf("You: ");
        if (!fgets(buffer, sizeof(buffer), stdin)) break;
        if (buffer[0] == 'q' || buffer[0] == 'Q') {
            write(socket_fd, "q\n", 2);
            break;
        }
        write(socket_fd, buffer, strlen(buffer));

        memset(buffer, 0, sizeof(buffer));
        ssize_t r = read(socket_fd, buffer, sizeof(buffer) - 1);
        if (r <= 0) break;
        buffer[r] = '\0';
        if (buffer[0] == 'q' || buffer[0] == 'Q') {
            printf("Server requested quit\n");
            break;
        }
        printf("Server: %s", buffer);
    }

    close(socket_fd);
    return 0;
}
