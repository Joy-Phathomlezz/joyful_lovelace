#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/time.h>

#define BUFSIZE 4096

void send_file(int client_sock, char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        write(client_sock, "NOFILE", 6);
        return;
    }
    write(client_sock, "READY", 5);

    char buf[BUFSIZE];
    ssize_t n;
    while ((n = fread(buf, 1, BUFSIZE, fp)) > 0) {
        if (write(client_sock, buf, n) < 0) break;
    }
    fclose(fp);
}

void recv_file(int client_sock, char *filename) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) return;
    char buf[BUFSIZE];
    ssize_t n;
    while ((n = read(client_sock, buf, BUFSIZE)) > 0) {
        if (strncmp(buf, "ENDOFFILE", 9) == 0) break;
        fwrite(buf, 1, n, fp);
        if (n < BUFSIZE) break; // Assume last chunk
    }
    fclose(fp);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        return 1;
    }
    int port = atoi(argv[1]);
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); exit(1); }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind");
        exit(1);
    }
    listen(server_fd, 5);
    printf("Server listening on port %d\n", port);

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    while (1) {
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) { perror("accept"); continue; }
        printf("Client connected.\n");

        char option[16], filename[256];
        read(client_fd, option, sizeof(option));
        read(client_fd, filename, sizeof(filename));

        struct timeval start, end;
        gettimeofday(&start, NULL);

        if (strncmp(option, "UPLOAD", 6) == 0) {
            recv_file(client_fd, filename);
            gettimeofday(&end, NULL);
            double transfer_time = ((end.tv_sec - start.tv_sec) * 1000.0 +
                                    (end.tv_usec - start.tv_usec) / 1000.0);
            printf("File %s uploaded. Transfer time: %.2f ms\n", filename, transfer_time);
        } else if (strncmp(option, "DOWNLOAD", 8) == 0) {
            send_file(client_fd, filename);
            gettimeofday(&end, NULL);
            double transfer_time = ((end.tv_sec - start.tv_sec) * 1000.0 +
                                    (end.tv_usec - start.tv_usec) / 1000.0);
            printf("File %s sent. Transfer time: %.2f ms\n", filename, transfer_time);
        }
        close(client_fd);
    }
    close(server_fd);
    return 0;
}
