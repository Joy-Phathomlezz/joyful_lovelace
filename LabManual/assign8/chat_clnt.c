#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define BUF_SIZE 1024

int sockfd;
int clientNum;

void *recv_thread(void *arg) {
    char buf[BUF_SIZE];
    while (1) {
        ssize_t len = recv(sockfd, buf, sizeof(buf)-1, 0);
        if (len > 0) {
            buf[len] = '\0';
            printf("%s", buf);
            fflush(stdout);
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <server_ip> <port> <clientNum>\n", argv[0]);
        return 1;
    }

    char *server_ip = argv[1];
    int port = atoi(argv[2]);
    clientNum = atoi(argv[3]);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) { perror("socket"); exit(1); }

    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    inet_pton(AF_INET, server_ip, &servaddr.sin_addr);

    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("connect"); exit(1);
    }

    pthread_t tid;
    pthread_create(&tid, NULL, recv_thread, NULL);

    char buf[BUF_SIZE], msg[BUF_SIZE + 32];
    while (fgets(buf, sizeof(buf), stdin)) {
        snprintf(msg, sizeof(msg), "Client%d: %s", clientNum, buf);
        send(sockfd, msg, strlen(msg), 0);
    }
    close(sockfd);
    return 0;
}
