#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/if_ether.h>
#include <linux/if_packet.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>

#define BUF_SIZE 2048

void print_ethernet_header(const u_char *packet) {
    struct ether_header *eth = (struct ether_header *)packet;
    printf("Ethernet Src: %02x:%02x:%02x:%02x:%02x:%02x ",
           eth->ether_shost[0], eth->ether_shost[1], eth->ether_shost[2],
           eth->ether_shost[3], eth->ether_shost[4], eth->ether_shost[5]);
    printf("Dst: %02x:%02x:%02x:%02x:%02x:%02x\n",
           eth->ether_dhost[0], eth->ether_dhost[1], eth->ether_dhost[2],
           eth->ether_dhost[3], eth->ether_dhost[4], eth->ether_dhost[5]);
    printf("Type: 0x%04x\n", ntohs(eth->ether_type));
}

void print_ip_header(const u_char *packet) {
    struct ether_header *eth = (struct ether_header *)packet;
    struct iphdr *ip = (struct iphdr *)(packet + sizeof(struct ether_header));
    
    printf("IP Version: %d, Header Len: %d words\n", ip->version, ip->ihl);
    printf("Src IP: %s ", inet_ntoa(*(struct in_addr*)&ip->saddr));
    printf("Dst IP: %s\n", inet_ntoa(*(struct in_addr*)&ip->daddr));
    printf("Protocol: %d (%s)\n", ip->protocol,
           ip->protocol == IPPROTO_TCP ? "TCP" :
           ip->protocol == IPPROTO_UDP ? "UDP" : "Other");
}

void print_tcp_header(const u_char *packet) {
    struct ether_header *eth = (struct ether_header *)packet;
    struct iphdr *ip = (struct iphdr *)(packet + sizeof(struct ether_header));
    struct tcphdr *tcp = (struct tcphdr *)(packet + sizeof(struct ether_header) + 
                                          (ip->ihl * 4));
    
    printf("TCP Src Port: %d, Dst Port: %d\n", 
           ntohs(tcp->source), ntohs(tcp->dest));
}

void print_packet_data(const u_char *data, int len, int offset) {
    printf("Data (%d bytes):\n", len);
    for (int i = 0; i < len && i < 64; i++) {
        printf("%02x ", data[offset + i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
    printf("\n");
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <interface> (e.g., eth0)\n", argv[0]);
        exit(1);
    }
    
    // Create PACKET socket (TCP/IP socket library extension)
    int sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sockfd < 0) {
        perror("socket(AF_PACKET)");
        printf("Run as root: sudo ./packet_sniffer eth0\n");
        exit(1);
    }
    
    // Bind to specific interface
    struct ifreq ifr;
    strncpy(ifr.ifr_name, argv[1], IFNAMSIZ);
    if (ioctl(sockfd, SIOCGIFINDEX, &ifr) < 0) {
        perror("ioctl SIOCGIFINDEX");
        exit(1);
    }
    
    printf("Listening on %s for Ethernet packets...\n", argv[1]);
    printf("Ctrl+C to stop\n\n");
    
    char buffer[BUF_SIZE];
    while (1) {
        ssize_t len = recvfrom(sockfd, buffer, BUF_SIZE, 0, NULL, NULL);
        if (len < 0) {
            perror("recvfrom");
            continue;
        }
        
        // Skip non-IP packets
        struct ether_header *eth = (struct ether_header *)buffer;
        if (ntohs(eth->ether_type) != ETHERTYPE_IP) continue;
        
        printf("\n=== PACKET CAPTURED (%d bytes) ===\n", (int)len);
        print_ethernet_header(buffer);
        print_ip_header(buffer);
        
        struct iphdr *ip = (struct iphdr *)(buffer + sizeof(struct ether_header));
        if (ip->protocol == IPPROTO_TCP) {
            print_tcp_header(buffer);
        }
        
        // Print first 64 bytes of data
        int ip_start = sizeof(struct ether_header) + (ip->ihl * 4);
        print_packet_data(buffer, len, ip_start);
        printf("================================\n");
    }
    
    close(sockfd);
    return 0;
}
