#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFLEN 1024
#define MAX_FRUITS 10
#define MAX_CUSTOMERS 50

typedef struct {
    char name[32];
    int quantity;
    char last_sold[64];
} Fruit;

typedef struct {
    char ip[32];
    int port;
} Customer;

Fruit fruits[MAX_FRUITS] = {
    {"apple", 10, "-"}, {"banana", 8, "-"}, {"mango", 6, "-"}
};
int num_fruits = 3;

Customer customers[MAX_CUSTOMERS];
int unique_customers = 0;

// Helper: update customer list if new
void add_customer(struct sockaddr_in *cliaddr) {
    char ip[32];
    strcpy(ip, inet_ntoa(cliaddr->sin_addr));
    int port = ntohs(cliaddr->sin_port);
    // Check if present
    for (int i = 0; i < unique_customers; ++i)
        if (strcmp(customers[i].ip, ip)==0 && customers[i].port == port)
            return;
    // Else add
    strcpy(customers[unique_customers].ip, ip);
    customers[unique_customers].port = port;
    unique_customers++;
}

// Helper: get current time string
void now_str(char *buf, size_t buflen) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    snprintf(buf, buflen, "%04d-%02d-%02d %02d:%02d:%02d",
        tm.tm_year+1900,tm.tm_mon+1,tm.tm_mday,tm.tm_hour,tm.tm_min,tm.tm_sec);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]); exit(1); }
    int port = atoi(argv[1]);

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd<0) { perror("socket"); exit(1);}
    struct sockaddr_in servaddr, cliaddr;
    memset(&servaddr, 0,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(port);
    if(bind(sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr))<0){
        perror("bind"); exit(1);}
    printf("Server ready on port %d\n", port);

    socklen_t clen = sizeof(cliaddr);
    char buf[BUFLEN];
    for(;;){
        ssize_t n = recvfrom(sockfd, buf, sizeof(buf)-1, 0,
                             (struct sockaddr*)&cliaddr, &clen);
        if(n<0){perror("recvfrom"); continue;}
        buf[n]=0;
        add_customer(&cliaddr);

        char fruit[32]; int qty;
        int req = sscanf(buf, "%31s %d", fruit, &qty);
        char reply[BUFLEN];

        if (req != 2) {
            snprintf(reply, sizeof(reply), "FORMAT ERROR: send '<fruit> <qty>'");
            goto RESPOND;
        }

        int found = 0;
        for(int f=0;f<num_fruits;f++){
            if(strcmp(fruits[f].name, fruit)==0){
                found=1;
                if(fruits[f].quantity >= qty) {
                    fruits[f].quantity -= qty;
                    now_str(fruits[f].last_sold, sizeof(fruits[f].last_sold));
                    snprintf(reply, sizeof(reply),
                            "Success: %d %s(s) sold. Remaining: %d (Last sold %s)",
                            qty,fruit,fruits[f].quantity,fruits[f].last_sold);
                } else {
                    snprintf(reply, sizeof(reply),
                            "Regret: Only %d %s(s) left. Transaction failed.",
                            fruits[f].quantity, fruit);
                }
                goto RESPOND;
            }
        }
        if (!found)
            snprintf(reply, sizeof(reply), "No such fruit in stock.");

RESPOND:
        // Add customer info & count
        char custlist[BUFLEN] = "Transacted customer(s): ";
        for (int i = 0; i < unique_customers; ++i) {
            char temp[64];
            snprintf(temp, sizeof(temp), "[%s:%d] ",
                customers[i].ip, customers[i].port);
            strncat(custlist, temp, sizeof(custlist) - strlen(custlist) - 1);
        }
        snprintf(buf, sizeof(buf), "%s\n%s\nTotal unique customers: %d",
            reply, custlist, unique_customers);

        sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr*)&cliaddr, clen);
        printf("%s\n", buf);
    }

    close(sockfd);
    return 0;
}
