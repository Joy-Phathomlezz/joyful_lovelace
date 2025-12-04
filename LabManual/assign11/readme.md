# CN Lab - Assignment 11

## Objective
Using RAW sockets to generate TCP flooding attack.

## Exercise
Write a RAW socket program to generate TCP SYN flood based DDoS attack towards an IP address. Take four mininet hosts as agent devices.

## Implementation

### TCP SYN Flood ([syn_flood.c](file:///c%3A/Users/spysc/OneDrive/Desktop/CSM24065_CNAssignment/assignment11/syn_flood.c))
- Creates TCP SYN flood attack using raw sockets
- Uses four spoofed IPs as agent devices (10.0.0.3, 10.0.0.4, 10.0.0.5, 10.0.0.6)
- Sends continuous SYN packets to target IP
- Randomizes source ports for each packet
- Implements proper IP and TCP header construction

## Compilation
```bash
make
```

Note: Root privileges are required to create raw sockets.

## Usage
Mininet: sudo mn --topo linear,6

```bash
sudo ./syn_flood 10.0.0.2
```

## Mininet Setup
1. Create 6 hosts in Mininet:
   - 1 attacker (running the SYN flood program)
   - 1 victim (target IP: 10.0.0.2)
   - 4 agent devices (spoofed IPs: 10.0.0.3, 10.0.0.4, 10.0.0.5, 10.0.0.6)

2. Run the SYN flood program on the attacker host targeting the victim IP

## Learning Outcomes
1. Use of RAW sockets to generate SYN based flooding attack.
2. Understanding the pattern of TCP control messages for DDoS attack.