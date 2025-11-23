#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFLEN 512

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    int port = atoi(argv[1]);
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) { perror("socket"); exit(1); }

    struct sockaddr_in servaddr, cliaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        perror("bind"); exit(1);
    }
    printf("UDP Calc Server listening on port %d\n", port);

    char buffer[BUFLEN];
    socklen_t clen;
    while (1) {
        clen = sizeof(cliaddr);
        ssize_t n = recvfrom(sockfd, buffer, BUFLEN-1, 0, (struct sockaddr*)&cliaddr, &clen);
        if (n < 0) { perror("recvfrom"); continue; }
        buffer[n] = '\0';

        char op[16];
        double x, y;
        int num_read = sscanf(buffer, "%lf %lf %15s", &x, &y, op);
        printf("Received: x=%.4f y=%.4f op=%s from %s:%d\n",
            x, y, op, inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));

        double result = 0.0;

        if (num_read == 3 && strcmp(op, "sin") == 0) {
            result = sin(x) - sin(y);
        } else {
            snprintf(buffer, BUFLEN, "ERR");
            sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&cliaddr, clen);
            continue;
        }

        snprintf(buffer, BUFLEN, "%.6f", result);
        sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&cliaddr, clen);
        printf("Result sent: %s\n", buffer);
    }

    close(sockfd);
    return 0;
}
