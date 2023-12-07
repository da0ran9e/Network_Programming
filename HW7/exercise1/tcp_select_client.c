#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "tcp_server_client.h"

/*
* Create a new client
* [IN] port: what port to use
* [OUT] client_sock: client's socket
*/
int initialize_client(int port, char * inputAddr) {
	int client_sock;
	struct sockaddr_in server_addr; /* server's address information */

	//Step 1: Construct socket
	client_sock = socket(AF_INET, SOCK_STREAM, 0);
	
	//Step 2: Specify server address
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr(inputAddr);
	
	//Step 3: Request to connect server
	if(connect(client_sock, (struct sockaddr*)&server_addr, sizeof(struct sockaddr)) < 0) {
		printf("Error! Can not connect to sever! Client exit imediately!\n");
		close(client_sock);
		exit(0);
	}

	return client_sock;
}

void run_client(int client_sock) {
	//Step 4: Communicate with server
	printf("Client started.\n");
	client_echo(client_sock);
}

void cleanup_client(int client_sock) {
	//Step 5: Close socket
	close(client_sock);
}

int main(int argc, char *argv[]){
	if (argc != 3) {
		printf("Usage: %s IPAddress PortNumber\n", argv[0]);
		exit(1);
	}

	int client_sock;

	client_sock = initialize_client(atoi(argv[2]), argv[1]);

	run_client(client_sock);
	cleanup_client(client_sock);
	
	return 0;
}
