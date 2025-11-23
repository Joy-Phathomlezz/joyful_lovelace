#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define PACKET_SIZE 64

unsigned short checksum(void *b, int len) {
    unsigned short *buf = b;
    unsigned int sum = 0;
    unsigned short result;
    for (sum = 0; len > 1; len -= 2)
        sum += *buf++;
    if (len == 1)
        sum += *(unsigned char *) buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <target_ip>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    char *dest_ip = argv[1];
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in target;
    target.sin_family = AF_INET;
    if (inet_pton(AF_INET, dest_ip, &target.sin_addr) != 1) {
        perror("inet_pton");
        exit(EXIT_FAILURE);
    }

    unsigned char packet[PACKET_SIZE];
    memset(packet, 0, PACKET_SIZE);

    struct icmphdr *icmp = (struct icmphdr*) packet;
    icmp->type = ICMP_TIMESTAMP;       // Type 13
    icmp->code = 0;
    icmp->un.echo.id = htons(getpid() & 0xFFFF);
    icmp->un.echo.sequence = htons(1);

    // Fill ICMP timestamp fields (standard: originate, receive, transmit)
    uint32_t *ts = (uint32_t *) (packet + sizeof(struct icmphdr));
    struct timeval tv;
    gettimeofday(&tv, NULL);
    uint32_t ms_since_midnight = (tv.tv_sec % 86400) * 1000 + tv.tv_usec / 1000;

    ts[0] = htonl(ms_since_midnight); // originate
    ts[1] = htonl(0);                // receive set to 0 in request
    ts[2] = htonl(0);                // transmit set to 0 in request

    // Calculate ICMP checksum
    icmp->checksum = 0;
    icmp->checksum = checksum(packet, sizeof(struct icmphdr) + 12);

    ssize_t sent = sendto(sockfd, packet, sizeof(struct icmphdr) + 12, 0,
                         (struct sockaddr*)&target, sizeof(target));
    if (sent < 0) {
        perror("sendto");
    } else {
        printf("ICMP Timestamp request sent to %s (%zd bytes).\n", dest_ip, sent);
    }

    close(sockfd);
    return 0;
}
