#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pcap.h>
#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <time.h>

#define MAX_PACKETS 1000

typedef struct {
    struct timeval timestamp;
    int protocol_l2;
    int protocol_l3;
    int protocol_l4;
} PacketInfo;

PacketInfo packets[MAX_PACKETS];
int packet_count = 0;

void analyze_packet(struct pcap_pkthdr *header, const u_char *packet) {
    if (packet_count >= MAX_PACKETS) return;
    
    // Store timestamp
    packets[packet_count].timestamp = header->ts;
    
    // Analyze Ethernet header (L2)
    struct ether_header *eth_header = (struct ether_header*)packet;
    packets[packet_count].protocol_l2 = ntohs(eth_header->ether_type);
    
    // Analyze IP header (L3)
    struct ip *ip_header = (struct ip*)(packet + sizeof(struct ether_header));
    packets[packet_count].protocol_l3 = ip_header->ip_p;
    
    // Analyze transport layer (L4)
    packets[packet_count].protocol_l4 = 0;
    if (ip_header->ip_p == IPPROTO_TCP) {
        packets[packet_count].protocol_l4 = IPPROTO_TCP;
    } else if (ip_header->ip_p == IPPROTO_UDP) {
        packets[packet_count].protocol_l4 = IPPROTO_UDP;
    } else if (ip_header->ip_p == IPPROTO_ICMP) {
        packets[packet_count].protocol_l4 = IPPROTO_ICMP;
    }
    
    packet_count++;
}

void print_time_diagram() {
    printf("Time Diagram of Packet Protocols:\n");
    printf("================================\n");
    
    for (int i = 0; i < packet_count; i++) {
        struct tm *timeinfo = localtime(&packets[i].timestamp.tv_sec);
        char time_str[20];
        strftime(time_str, sizeof(time_str), "%H:%M:%S", timeinfo);
        
        printf("[%s.%06ld] ", time_str, packets[i].timestamp.tv_usec);
        
        // Print L2 protocol
        switch (packets[i].protocol_l2) {
            case ETHERTYPE_IP:
                printf("L2:Ethernet ");
                break;
            case ETHERTYPE_ARP:
                printf("L2:ARP ");
                break;
            default:
                printf("L2:Unknown(0x%04x) ", packets[i].protocol_l2);
                break;
        }
        
        // Print L3 protocol
        switch (packets[i].protocol_l3) {
            case IPPROTO_IP:
                printf("L3:IP ");
                break;
            case IPPROTO_ICMP:
                printf("L3:ICMP ");
                break;
            case IPPROTO_TCP:
                printf("L3:TCP ");
                break;
            case IPPROTO_UDP:
                printf("L3:UDP ");
                break;
            default:
                printf("L3:Unknown(%d) ", packets[i].protocol_l3);
                break;
        }
        
        // Print L4 protocol
        switch (packets[i].protocol_l4) {
            case IPPROTO_TCP:
                printf("L4:TCP");
                break;
            case IPPROTO_UDP:
                printf("L4:UDP");
                break;
            case IPPROTO_ICMP:
                printf("L4:ICMP");
                break;
            default:
                printf("L4:None");
                break;
        }
        
        printf("\n");
    }
}

void list_protocols() {
    printf("\nUnique Protocols Extracted:\n");
    printf("==========================\n");
    
    // L2 Protocols
    printf("L2 (Data Link Layer) Protocols:\n");
    int l2_ethernet = 0, l2_arp = 0;
    for (int i = 0; i < packet_count; i++) {
        if (packets[i].protocol_l2 == ETHERTYPE_IP) l2_ethernet = 1;
        if (packets[i].protocol_l2 == ETHERTYPE_ARP) l2_arp = 1;
    }
    if (l2_ethernet) printf("  - Ethernet (0x0800)\n");
    if (l2_arp) printf("  - ARP (0x0806)\n");
    
    // L3 Protocols
    printf("L3 (Network Layer) Protocols:\n");
    int l3_ip = 0, l3_icmp = 0, l3_tcp = 0, l3_udp = 0;
    for (int i = 0; i < packet_count; i++) {
        if (packets[i].protocol_l3 == IPPROTO_IP) l3_ip = 1;
        if (packets[i].protocol_l3 == IPPROTO_ICMP) l3_icmp = 1;
        if (packets[i].protocol_l3 == IPPROTO_TCP) l3_tcp = 1;
        if (packets[i].protocol_l3 == IPPROTO_UDP) l3_udp = 1;
    }
    if (l3_ip) printf("  - IP (Protocol 0)\n");
    if (l3_icmp) printf("  - ICMP (Protocol 1)\n");
    if (l3_tcp) printf("  - TCP (Protocol 6)\n");
    if (l3_udp) printf("  - UDP (Protocol 17)\n");
    
    // L4 Protocols
    printf("L4 (Transport Layer) Protocols:\n");
    int l4_tcp = 0, l4_udp = 0, l4_icmp = 0;
    for (int i = 0; i < packet_count; i++) {
        if (packets[i].protocol_l4 == IPPROTO_TCP) l4_tcp = 1;
        if (packets[i].protocol_l4 == IPPROTO_UDP) l4_udp = 1;
        if (packets[i].protocol_l4 == IPPROTO_ICMP) l4_icmp = 1;
    }
    if (l4_tcp) printf("  - TCP\n");
    if (l4_udp) printf("  - UDP\n");
    if (l4_icmp) printf("  - ICMP\n");
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <pcap_file>\n", argv[0]);
        return 1;
    }
    
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *handle = pcap_open_offline(argv[1], errbuf);
    
    if (handle == NULL) {
        fprintf(stderr, "Error opening pcap file: %s\n", errbuf);
        return 1;
    }
    
    struct pcap_pkthdr header;
    const u_char *packet;
    
    printf("Analyzing packets from %s\n\n", argv[1]);
    
    // Process packets
    while ((packet = pcap_next(handle, &header)) != NULL) {
        analyze_packet(&header, packet);
    }
    
    // Print results
    print_time_diagram();
    list_protocols();
    
    pcap_close(handle);
    return 0;
}