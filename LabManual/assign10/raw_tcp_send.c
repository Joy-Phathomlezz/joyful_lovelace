#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#define SOURCE_PORT 12345
#define DEST_PORT 80
#define PACKET_SIZE 4096

unsigned short checksum(unsigned short *ptr, int nbytes) {
    register long sum;
    unsigned short oddbyte;
    register short answer;

    sum = 0;
    while (nbytes > 1) {
        sum += *ptr++;
        nbytes -= 2;
    }

    if (nbytes == 1) {
        oddbyte = 0;
        *((u_char*)&oddbyte) = *(u_char*)ptr;
        sum += oddbyte;
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum = sum + (sum >> 16);
    answer = (short)~sum;

    return(answer);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <target_ip> <roll_number>\n", argv[0]);
        return -1;
    }

    int sockfd;
    char packet[PACKET_SIZE];
    struct iphdr *iph = (struct iphdr *) packet;
    struct tcphdr *tcph = (struct tcphdr *) (packet + sizeof(struct iphdr));
    struct sockaddr_in dest;
    
    // Zero out the packet buffer
    memset(packet, 0, PACKET_SIZE);
    
    // Create raw socket
    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sockfd < 0) {
        perror("Failed to create raw socket");
        exit(EXIT_FAILURE);
    }
    
    // Set destination
    dest.sin_family = AF_INET;
    dest.sin_port = htons(DEST_PORT);
    dest.sin_addr.s_addr = inet_addr(argv[1]);
    
    // Fill IP header
    iph->ihl = 5;
    iph->version = 4;
    iph->tos = 0;
    iph->tot_len = sizeof(struct iphdr) + sizeof(struct tcphdr) + strlen(argv[2]);
    iph->id = htonl(54321);
    iph->frag_off = 0;
    iph->ttl = 255;
    iph->protocol = IPPROTO_TCP;
    iph->check = 0;
    iph->saddr = inet_addr("10.0.0.1");
    iph->daddr = dest.sin_addr.s_addr;
    
    // IP checksum
    iph->check = checksum((unsigned short *) packet, iph->tot_len);
    
    // Fill TCP header
    tcph->source = htons(SOURCE_PORT);
    tcph->dest = htons(DEST_PORT);
    tcph->seq = 0;
    tcph->ack_seq = 0;
    tcph->doff = 5;
    tcph->fin = 0;
    tcph->syn = 1;
    tcph->rst = 0;
    tcph->psh = 0;
    tcph->ack = 0;
    tcph->urg = 0;
    tcph->window = htons(5840);
    tcph->check = 0;
    tcph->urg_ptr = 0;
    
    // Add roll number as payload
    char *payload = packet + sizeof(struct iphdr) + sizeof(struct tcphdr);
    strcpy(payload, argv[2]);
    
    // Send the packet
    if (sendto(sockfd, packet, iph->tot_len, 0, (struct sockaddr *)&dest, sizeof(dest)) < 0) {
        perror("Failed to send packet");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    
    printf("TCP packet sent to %s with roll number %s\n", argv[1], argv[2]);
    
    close(sockfd);
    return 0;
}