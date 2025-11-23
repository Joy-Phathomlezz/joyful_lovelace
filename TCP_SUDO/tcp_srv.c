
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{


	// socket 
	int srv_fd = socket(AF_INET,SOCK_STREAM,0);


	// bind 
	struct sockaddr_in srv_addr;

	srv_addr.sin_family = AF_INET;
	srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	srv_addr.sin_port = htons(6969);

	bind(srv_fd, (struct sockaddr*) &srv_addr , sizeof(srv_addr));

	// listen
	int nclient = 1;
	listen(srv_fd, nclient);

	// accept
	struct sockaddr_in clnt_addr;
	socklen_t clnt_addrlen = sizeof(clnt_addr);
	int clnt_fd = accept(srv_fd,(struct sockaddr*) &clnt_addr,&clnt_addrlen);
	
	//--operation

	// close
	close(srv_fd);
	close(clnt_fd);
	return 0;
}