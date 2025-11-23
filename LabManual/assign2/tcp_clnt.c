#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUF_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 5) {
        printf("Usage: %s <server_ip> <port> <fruit> <qty>\n", argv[0]);
        exit(1);
    }
    char *server_ip = argv[1];
    int port = atoi(argv[2]);
    char *fruit = argv[3];
    int qty = atoi(argv[4]);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) { perror("socket"); exit(1);}
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    inet_pton(AF_INET, server_ip, &servaddr.sin_addr);

    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("connect"); exit(1);
    }

    char buf[BUF_SIZE];
    snprintf(buf, sizeof(buf), "%s %d", fruit, qty);
    write(sockfd, buf, strlen(buf));
    printf("Sent request: '%s %d'\n", fruit, qty);

    int n = read(sockfd, buf, BUF_SIZE-1);
    if (n > 0) {
        buf[n] = 0;
        printf("Server reply:\n%s", buf);
    }
    close(sockfd);
    return 0;
}
