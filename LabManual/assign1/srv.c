#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>

#define PORT 5555
#define BUFLEN 128

int main() {
    int servfd = socket(AF_INET, SOCK_STREAM, 0);
    if (servfd < 0) { perror("socket"); exit(1); }

    struct sockaddr_in servaddr = {0}, cliaddr = {0};
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    if (bind(servfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind"); exit(1);
    }
    if (listen(servfd, 1) < 0) { perror("listen"); exit(1); }
    printf("Server ready on port %d. Waiting for connection...\n", PORT);

    socklen_t clen = sizeof(cliaddr);
    int connfd = accept(servfd, (struct sockaddr *)&cliaddr, &clen);
    if (connfd < 0) { perror("accept"); exit(1); }

    char buf[BUFLEN];
    int n = read(connfd, buf, sizeof(buf) - 1);
    if (n > 0) {
        buf[n] = 0;
        printf("Received: %s\n", buf);
        if (strcmp(buf, "Hi\n") == 0 || strcmp(buf, "Hi") == 0) {
            write(connfd, "Hello\n", 6);
            printf("Sent: Hello\n");
        } else {
            write(connfd, "??\n", 3);
        }
    }
    close(connfd);
    close(servfd);
    return 0;
}
