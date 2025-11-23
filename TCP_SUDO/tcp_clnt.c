#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
    //socket()
    int clnt_fd = socket(AF_INET, SOCK_STREAM, 0);

    //connet()
    struct sockaddr_in srv_addr;
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_addr.s_addr = inet_addr("10.0.0.1");
    srv_addr.sin_port = htons(6969);

    connect(clnt_fd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));

    //close()
    close(clnt_fd);
    return 0;
}
