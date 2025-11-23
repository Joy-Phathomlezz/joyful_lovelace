#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFLEN 512

int main(int argc, char *argv[]) {
    if (argc != 5) {
        printf("Usage: %s <server_ip> <port> <x> <y>\n", argv[0]);
        exit(1);
    }

    char *server_ip = argv[1];
    int port = atoi(argv[2]);
    double x = atof(argv[3]);
    double y = atof(argv[4]);

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) { perror("socket"); exit(1); }

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    inet_pton(AF_INET, server_ip, &servaddr.sin_addr);

    char buffer[BUFLEN];
    snprintf(buffer, BUFLEN, "%.8f %.8f sin", x, y);

    ssize_t sent = sendto(sockfd, buffer, strlen(buffer), 0,
                          (struct sockaddr*)&servaddr, sizeof(servaddr));
    if (sent < 0) { perror("sendto"); exit(1); }
    printf("Sent: %s\n", buffer);

    // Wait for reply
    ssize_t n = recvfrom(sockfd, buffer, BUFLEN-1, 0, NULL, NULL);
    if (n < 0) { perror("recvfrom"); exit(1);}
    buffer[n] = '\0';
    printf("Result from server: %s\n", buffer);

    close(sockfd);
    return 0;
}
