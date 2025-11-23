#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_CLIENTS 10
#define BUF_SIZE 1024

int clients[MAX_CLIENTS];
int client_count = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
FILE *logfile;

void broadcast(const char *msg, int skip_fd) {
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < client_count; i++) {
        if (clients[i] != skip_fd) {
            send(clients[i], msg, strlen(msg), 0);
        }
    }
    pthread_mutex_unlock(&mutex);
}

void *handle_client(void *arg) {
    int client_fd = *(int*)arg;
    char buf[BUF_SIZE];

    while (1) {
        ssize_t len = recv(client_fd, buf, sizeof(buf)-1, 0);
        if (len <= 0) break;
        buf[len] = '\0';

        pthread_mutex_lock(&mutex);
        fprintf(logfile, "%s", buf); fflush(logfile);
        pthread_mutex_unlock(&mutex);

        broadcast(buf, client_fd);
        printf("Message: %s", buf);
    }

    // Remove client
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < client_count; i++) {
        if (clients[i] == client_fd) {
            for (int j = i; j < client_count-1; j++) clients[j] = clients[j+1];
            client_count--;
            break;
        }
    }
    pthread_mutex_unlock(&mutex);
    close(client_fd);
    printf("Client disconnected.\n");
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]); return 1;
    }

    logfile = fopen("chatlog.txt", "a");
    if (!logfile) { perror("logfile"); exit(1); }

    int port = atoi(argv[1]), server_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in servaddr, cliaddr;
    if (server_fd < 0) { perror("socket"); exit(1);}
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind"); exit(1);
    }
    listen(server_fd, MAX_CLIENTS);
    printf("Chat server listening on port %d\n", port);

    while (1) {
        socklen_t len = sizeof(cliaddr);
        int client_fd = accept(server_fd, (struct sockaddr *)&cliaddr, &len);
        if (client_fd < 0) { perror("accept"); continue; }

        pthread_mutex_lock(&mutex);
        if (client_count < MAX_CLIENTS) {
            clients[client_count++] = client_fd;
            pthread_t tid;
            pthread_create(&tid, NULL, handle_client, &clients[client_count-1]);
            pthread_detach(tid);
            printf("New client connected: %s:%d\n",
                   inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
        } else {
            close(client_fd); // Too many clients
        }
        pthread_mutex_unlock(&mutex);
    }

    fclose(logfile);
    close(server_fd);
    return 0;
}
