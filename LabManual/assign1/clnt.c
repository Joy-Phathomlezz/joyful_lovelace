#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 5555
#define BUFLEN 128

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <server_ip>\n", argv[0]);
        exit(1);
    }
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) { perror("socket"); exit(1); }

    struct sockaddr_in servaddr = {0};
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) != 1) {
        perror("inet_pton"); exit(1);
    }

    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("connect"); exit(1);
    }

    char msg[] = "Hi\n";
    write(sockfd, msg, strlen(msg));
    char buf[BUFLEN];
    int n = read(sockfd, buf, BUFLEN-1);
    if (n > 0) {
        buf[n] = 0;
        printf("Server replied: %s", buf);
    }

    close(sockfd);
    return 0;
}
