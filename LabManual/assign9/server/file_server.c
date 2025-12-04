#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define BUF_SIZE 1024

void send_file(int sock, char *filename) {
    char buffer[BUF_SIZE];
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        strcpy(buffer, "FILE_NOT_FOUND");
        send(sock, buffer, sizeof(buffer), 0);
        return;
    }

    // File exists
    strcpy(buffer, "FILE_FOUND");
    send(sock, buffer, sizeof(buffer), 0);

    // Start timing
    clock_t start = clock();

    while (!feof(fp)) {
        int n = fread(buffer, 1, BUF_SIZE, fp);
        send(sock, buffer, n, 0);
    }

    clock_t end = clock();
    double t = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Server: Time taken to send file = %f sec\n", t);

    fclose(fp);
}

void receive_file(int sock, char *filename) {
    char buffer[BUF_SIZE];
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        perror("File open error");
        return;
    }

    // Start timing
    clock_t start = clock();

    int n;
    while ((n = recv(sock, buffer, BUF_SIZE, 0)) > 0) {
        fwrite(buffer, 1, n, fp);
        if (n < BUF_SIZE) break;
    }

    clock_t end = clock();
    double t = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Server: Time taken to receive file = %f sec\n", t);

    fclose(fp);
}

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;

    server_sock = socket(AF_INET, SOCK_STREAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8000);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_sock, 5);

    printf("Server waiting for connection...\n");
    addr_size = sizeof(client_addr);
    client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_size);
    printf("Client connected.\n");

    char command[50];
    recv(client_sock, command, sizeof(command), 0);

    if (strcmp(command, "DOWNLOAD") == 0) {
        send_file(client_sock, "server_file.txt");
    } else if (strcmp(command, "UPLOAD") == 0) {
        receive_file(client_sock, "recv_client.txt");
    }

    close(client_sock);
    close(server_sock);
    return 0;
}