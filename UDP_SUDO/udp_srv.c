#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char const *argv[])
{
	// require port argument
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <port>\n", argv[0]);
		return 1;
	}

	//socket()
	int sock_fd = socket(AF_INET,SOCK_DGRAM,0);
	if (sock_fd < 0) {
		perror("socket");
		return 1;
	}

	//bind()
	struct sockaddr_in sock_addr;
	memset(&sock_addr, 0, sizeof(sock_addr));
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	sock_addr.sin_port = htons(atoi(argv[1]));
	if (bind(sock_fd,(struct sockaddr *) &sock_addr, sizeof(sock_addr)) < 0) {
		perror("bind");
		close(sock_fd);
		return 1;
	}


	char buffer[512];
	struct sockaddr_in clnt_addr;
	socklen_t clnt_addr_len = sizeof(clnt_addr);

	printf("Type q to quit: \n");
	while(1){
	//recvfrom()
		ssize_t recv_len = recvfrom(sock_fd,buffer,sizeof(buffer)-1,0,
							(struct sockaddr*)&clnt_addr,&clnt_addr_len);

		if (recv_len < 0) {
			perror("recvfrom");
			break;
		}

		if(recv_len == 1 && buffer[0] =='q'){
			break;
		}
		//to handle string communication 
		buffer[recv_len] = '\0';

		printf("peer : %s\n",buffer);

		
		//echo 
		const char *reply = "ACK";
		if (sendto(sock_fd,reply,strlen(reply),0,
				(struct sockaddr*)&clnt_addr,clnt_addr_len) < 0) {
			perror("sendto");
			break;
		}
		
	}

	//close()
	close(sock_fd);

	return 0;
}