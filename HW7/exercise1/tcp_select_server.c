#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// #include <netdb.h>
// #include <sys/wait.h>
// #include <errno.h>

#include "tcp_server_client.h"


/*
* Create a new server
* [IN] port: what port to use
* [OUT] listen_sock: file descriptors
*/
int initialize_server(int port) {
	int listen_sock;			/* file descriptors */
	struct sockaddr_in server;	/* server's address information */

	if ((listen_sock=socket(AF_INET, SOCK_STREAM, 0)) == -1 ) {	/* calls socket() */
		printf("socket() error\n");
		exit(1);
	}
	
	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = htonl(INADDR_ANY);	/* INADDR_ANY puts your IP address automatically */

	if(bind(listen_sock, (struct sockaddr*)&server, sizeof(server))==-1) { 
		perror("\nError bind: ");
		close(listen_sock);
		exit(1);
	}

	if(listen(listen_sock, BACKLOG) == -1){
		perror("\nError listen: ");
		close(listen_sock);
		exit(1);
	}

	printf("Server created.\n\n");
	return listen_sock;
}

void run_server(int listen_sock) {
	server_echo(listen_sock);
}

void cleanup_server(int listen_sock) {
	close(listen_sock);
}

int main(int argc, char *argv[]){
	if (argc != 2) {
		printf("Usage: %s PortNumber\n", argv[0]);
		exit(1);
	}
	
	int listen_sock; /* file descriptors */

	listen_sock = initialize_server(atoi(argv[1]));

	run_server(listen_sock);
	cleanup_server(listen_sock);
	
	return 0;
}