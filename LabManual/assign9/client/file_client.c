#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define BUF_SIZE 1024

void download_file(int sock) {
    char buffer[BUF_SIZE];

    recv(sock, buffer, BUF_SIZE, 0);
    if (!strcmp(buffer, "FILE_NOT_FOUND")) {
        printf("Client: File not found on server.\n");
        return;
    }

    FILE *fp = fopen("downloaded_from_server.txt", "wb");

    clock_t start = clock();

    int n;
    while ((n = recv(sock, buffer, BUF_SIZE, 0)) > 0) {
        fwrite(buffer, 1, n, fp);
        if (n < BUF_SIZE) break;
    }

    clock_t end = clock();
    double t = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Client: Time taken to download file = %f sec\n", t);

    fclose(fp);
}

void upload_file(int sock) {
    char buffer[BUF_SIZE];
    FILE *fp = fopen("client_file.txt", "rb");

    if (!fp) {
        printf("Client: File not found.\n");
        return;
    }

    clock_t start = clock();

    while (!feof(fp)) {
        int n = fread(buffer, 1, BUF_SIZE, fp);
        send(sock, buffer, n, 0);
    }

    clock_t end = clock();
    double t = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Client: Time taken to upload file = %f sec\n", t);

    fclose(fp);
}

int main() {
    int client_sock;
    struct sockaddr_in server_addr;

    client_sock = socket(AF_INET, SOCK_STREAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8000);
    server_addr.sin_addr.s_addr = inet_addr("10.0.0.1");  // Server IP in Mininet

    connect(client_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));

    int choice;
    printf("1. Download file from server\n2. Upload file to server\n");
    scanf("%d", &choice);

    if (choice == 1) {
        send(client_sock, "DOWNLOAD", 50, 0);
        download_file(client_sock);
    } else if (choice == 2) {
        send(client_sock, "UPLOAD", 50, 0);
        upload_file(client_sock);
    }

    close(client_sock);
    return 0;
}