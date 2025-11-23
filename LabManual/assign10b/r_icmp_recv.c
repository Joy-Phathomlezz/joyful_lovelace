#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>

#define BUFLEN 1024

int main() {
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    unsigned char buffer[BUFLEN];
    printf("Listening for ICMP Timestamp replies...\n");
    while (1) {
        ssize_t len = recvfrom(sockfd, buffer, BUFLEN, 0, NULL, NULL);
        if (len < 0) {
            perror("recvfrom");
            continue;
        }

        struct iphdr *ip = (struct iphdr*) buffer;
        int iphdrlen = ip->ihl * 4;
        struct icmphdr *icmp = (struct icmphdr*) (buffer + iphdrlen);

        if (icmp->type == ICMP_TIMESTAMPREPLY) { // Type 14
            uint32_t *ts = (uint32_t *) (buffer + iphdrlen + sizeof(struct icmphdr));
            printf("Received ICMP Timestamp Reply from %s\n", inet_ntoa(*(struct in_addr *) &ip->saddr));
            printf("ID: %u, Seq: %u\n", ntohs(icmp->un.echo.id), ntohs(icmp->un.echo.sequence));
            printf("Originate TS: %u\n", ntohl(ts[0]));
            printf("Receive   TS: %u\n", ntohl(ts[1]));
            printf("Transmit  TS: %u\n", ntohl(ts[2]));
            printf("\n");
        }
    }

    close(sockfd);
    return 0;
}
