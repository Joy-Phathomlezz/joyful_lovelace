#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

#define PCKT_LEN 8192

unsigned short checksum(void *b, int len)
{
    unsigned short *buf = b;
    unsigned int sum = 0;
    unsigned short result;

    for (; len > 1; len -= 2)
        sum += *buf++;
    if (len == 1)
        sum += *(unsigned char *)buf;

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);

    result = ~sum;
    return result;
}

int main()
{
    int sock;
    char buffer[PCKT_LEN];

    struct iphdr *ip = (struct iphdr *) buffer;
    struct icmphdr *icmp = (struct icmphdr *) (buffer + sizeof(struct iphdr));

    char dst_ip[20];
    printf("Enter target IP for ICMP Timestamp request: ");
    scanf("%s", dst_ip);

    memset(buffer, 0, PCKT_LEN);

    sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock < 0) {
        perror("Raw socket error");
        exit(1);
    }

    int one = 1;
    setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one));

    // ---------------- IP HEADER ----------------
    ip->ihl = 5;
    ip->version = 4;
    ip->tot_len = htons(sizeof(struct iphdr) + sizeof(struct icmphdr) + 12);
    ip->ttl = 64;
    ip->protocol = IPPROTO_ICMP;
    ip->saddr = inet_addr("1.2.3.4"); // fake source allowed
    ip->daddr = inet_addr(dst_ip);
    ip->check = checksum((unsigned short *) ip, sizeof(struct iphdr));

    // ---------------- ICMP TIMESTAMP ----------------
    icmp->type = 13;   // Timestamp Request
    icmp->code = 0;
    icmp->un.echo.id = htons(1);
    icmp->un.echo.sequence = htons(1);

    // Timestamp fields (originate, receive, transmit)
    unsigned long *timestamps = (unsigned long *)(buffer + sizeof(struct iphdr) + sizeof(struct icmphdr));
    timestamps[0] = htonl(123456);  // originate timestamp
    timestamps[1] = 0;              // receive timestamp (reply adds)
    timestamps[2] = 0;              // transmit timestamp

    icmp->checksum = 0;
    icmp->checksum = checksum(icmp, sizeof(struct icmphdr) + 12);

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = ip->daddr;

    if (sendto(sock, buffer, ntohs(ip->tot_len), 0,
               (struct sockaddr *)&sin, sizeof(sin)) < 0) {
        perror("Send failed");
    } else {
        printf("ICMP Timestamp Request sent to %s\n", dst_ip);
        printf("Capture packet in Wireshark using filter: icmp.type == 13\n");
    }

    close(sock);
    return 0;
}