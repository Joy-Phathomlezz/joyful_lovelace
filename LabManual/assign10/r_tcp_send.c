#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#define PCKT_LEN 4096

unsigned short checksum(unsigned short *buf, int len) {
    unsigned long sum = 0;
    while(len > 1) {
        sum += *buf++;
        len -= 2;
    }
    if(len) sum += *(unsigned char*)buf;
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return (unsigned short)(~sum);
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <source_ip> <dest_ip> <src_port> <dest_port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *src_ip = argv[1];
    char *dst_ip = argv[2];
    int src_port = atoi(argv[3]);
    int dst_port = atoi(argv[4]);

    char datagram[PCKT_LEN];
    memset(datagram, 0, PCKT_LEN);
    char data[] = "abcdefghijklmnopqrstuvwxyz";
    int datalen = strlen(data);

    struct iphdr *iph = (struct iphdr *) datagram;
    struct tcphdr *tcph = (struct tcphdr *) (datagram + sizeof(struct iphdr));
    char *payload = datagram + sizeof(struct iphdr) + sizeof(struct tcphdr);
    memcpy(payload, data, datalen);

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(dst_port);
    if (inet_pton(AF_INET, dst_ip, &sin.sin_addr) != 1) {
        perror("inet_pton");
        exit(EXIT_FAILURE);
    }

    // Fill IP Header
    iph->ihl = 5;
    iph->version = 4;
    iph->tos = 0;
    iph->tot_len = htons(sizeof(struct iphdr) + sizeof(struct tcphdr) + datalen);
    iph->id = htons(54321);
    iph->frag_off = 0;
    iph->ttl = 64;
    iph->protocol = IPPROTO_TCP;
    iph->saddr = inet_addr(src_ip);
    iph->daddr = sin.sin_addr.s_addr;
    iph->check = 0;
    iph->check = checksum((unsigned short *)datagram, iph->ihl * 4);

    // Fill TCP Header
    tcph->source = htons(src_port);
    tcph->dest = htons(dst_port);
    tcph->seq = htonl(0);
    tcph->ack_seq = 0;
    tcph->doff = 5;
    tcph->syn = 1;  // SYN flag
    tcph->window = htons(5840);
    tcph->check = 0; // For learning/demo
    tcph->urg_ptr = 0;

    int sd = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    int one = 1;
    if (setsockopt(sd, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
        perror("setsockopt");
        close(sd);
        exit(EXIT_FAILURE);
    }

    ssize_t sent_size = sendto(sd, datagram, sizeof(struct iphdr)+sizeof(struct tcphdr)+datalen,
                               0, (struct sockaddr *)&sin, sizeof(sin));
    if (sent_size < 0) {
        perror("sendto failed");
    } else {
        printf("Raw TCP packet sent (%zd bytes) to %s:%d\n", sent_size, dst_ip, dst_port);
    }

    close(sd);
    return 0;
}
