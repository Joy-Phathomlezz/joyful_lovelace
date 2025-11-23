#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFLEN 1024

int main(int argc, char *argv[]) {
    if (argc != 5) {
        printf("Usage: %s <server_ip> <port> <fruit> <qty>\n", argv[0]);
        exit(1);
    }
    char *server_ip = argv[1];
    int port = atoi(argv[2]);
    char *fruit = argv[3];
    int qty = atoi(argv[4]);

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) { perror("socket"); exit(1);}
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    inet_pton(AF_INET, server_ip, &servaddr.sin_addr);

    char buf[BUFLEN];
    snprintf(buf, sizeof(buf), "%s %d", fruit, qty);

    sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr*)&servaddr, sizeof(servaddr));
    printf("Sent request: '%s %d'\n", fruit, qty);

    ssize_t n = recvfrom(sockfd, buf, BUFLEN-1, 0, NULL, NULL);
    if (n < 0) {perror("recvfrom"); exit(1);}
    buf[n]=0;
    printf("Server reply:\n%s\n", buf);

    close(sockfd);
    return 0;
}
