#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/time.h>

#define BUFSIZE 4096

void upload_file(int sock, char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) { printf("File not found.\n"); return; }
    char buf[BUFSIZE];
    ssize_t n;
    while ((n = fread(buf, 1, BUFSIZE, fp)) > 0) {
        write(sock, buf, n);
    }
    write(sock, "ENDOFFILE", 9);
    fclose(fp);
}

void download_file(int sock, char *filename) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) { printf("Cannot create file.\n"); return; }
    char buf[BUFSIZE];
    ssize_t n;
    while ((n = read(sock, buf, BUFSIZE)) > 0) {
        if (strncmp(buf, "NOFILE", 6) == 0) {
            printf("Server: File not found.\n");
            break;
        }
        if (strncmp(buf, "READY", 5) == 0) {
            continue;
        }
        fwrite(buf, 1, n, fp);
        if (n < BUFSIZE) break;
    }
    fclose(fp);
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        printf("Usage: %s <server_IP> <port> <UPLOAD/DOWNLOAD> <filename>\n", argv[0]);
        return 1;
    }
    char *server_ip = argv[1];
    int port = atoi(argv[2]);
    char *option = argv[3];
    char *filename = argv[4];

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) { perror("socket"); exit(1); }
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    inet_pton(AF_INET, server_ip, &serv_addr.sin_addr);

    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect");
        exit(1);
    }
    write(sockfd, option, strlen(option) + 1);
    write(sockfd, filename, strlen(filename) + 1);

    struct timeval start, end;
    gettimeofday(&start, NULL);

    if (strncmp(option, "UPLOAD", 6) == 0) {
        upload_file(sockfd, filename);
        gettimeofday(&end, NULL);
        double transfer_time = ((end.tv_sec - start.tv_sec) * 1000.0 +
                                (end.tv_usec - start.tv_usec) / 1000.0);
        printf("Upload transfer time: %.2f ms\n", transfer_time);
    } else if (strncmp(option, "DOWNLOAD", 8) == 0) {
        download_file(sockfd, filename);
        gettimeofday(&end, NULL);
        double transfer_time = ((end.tv_sec - start.tv_sec) * 1000.0 +
                                (end.tv_usec - start.tv_usec) / 1000.0);
        printf("Download transfer time: %.2f ms\n", transfer_time);
    }
    close(sockfd);
    return 0;
}
