#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#define BUFLEN 4096

int main() {
    int sd = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sd < 0) { perror("socket"); exit(EXIT_FAILURE); }

    printf("Listening for TCP packets...\n");
    char buffer[BUFLEN];

    while (1) {
        struct sockaddr_in src_addr;
        socklen_t addrlen = sizeof(src_addr);
        ssize_t len = recvfrom(sd, buffer, BUFLEN, 0, (struct sockaddr *)&src_addr, &addrlen);
        if (len < 0) {
            perror("recvfrom");
            continue;
        }
        struct iphdr *iph = (struct iphdr *) buffer;
        if (iph->protocol != IPPROTO_TCP) continue;

        int iphdrlen = iph->ihl * 4;
        struct tcphdr *tcph = (struct tcphdr *)(buffer + iphdrlen);
        int tcphdrlen = tcph->doff * 4;
        char *payload = buffer + iphdrlen + tcphdrlen;
        int paylen = len - (iphdrlen + tcphdrlen);

        struct in_addr s_addr = {iph->saddr};
        struct in_addr d_addr = {iph->daddr};

        printf("From: %s:%d -> %s:%d\n",
               inet_ntoa(s_addr), ntohs(tcph->source),
               inet_ntoa(d_addr), ntohs(tcph->dest));
        printf("Payload (%d bytes): ", paylen);
        for (int i = 0; i < paylen; i++) {
            putchar(payload[i]);
        }
        putchar('\n');
    }

    close(sd);
    return 0;
}
