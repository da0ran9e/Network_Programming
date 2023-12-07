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
#include <poll.h>
#include <limits.h>

#include "tcp_server_client.h"
#include "read_accounts.h"


/*
* Create a new server
* [IN] port: what port to use
* [OUT] listen_sock: file descriptors
*/
int initialize_server(int port) {
	int listen_sock;			/* file descriptors */
	struct sockaddr_in server;	/* server's address information */

	if ((listen_sock=socket(AF_INET, SOCK_STREAM, 0)) == -1 ){	/* calls socket() */
		printf("socket() error\n");
		exit(1);
	}
	
	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = htonl(INADDR_ANY);	/* INADDR_ANY puts your IP address automatically */

	if(bind(listen_sock, (struct sockaddr*)&server, sizeof(server))==-1){ 
		perror("\nError bind: ");
		close(listen_sock);
		exit(1);
	}

	if(listen(listen_sock, BACKLOG) == -1){
		perror("\nError listen: ");
		close(listen_sock);
		exit(1);
	}

	initializeAccountsFromFile();

	printf("Server created.\n\n");
	return listen_sock;
}

void run_server(int listen_sock) {
	int nready, maxi, i, connfd, sockfd;

	socklen_t clilen;
	struct sockaddr_in cliaddr;
	
	const int OPEN_MAX = sysconf(_SC_OPEN_MAX);  // maximum number of opened files 
	int INFTIM = -1;

	struct pollfd clients[OPEN_MAX];
	// Implementation

	clients[0].fd = listen_sock;
	clients[0].events = POLLRDNORM;

	for (i = 1; i < OPEN_MAX; i++) {
		clients[i].fd = -1;     // -1 indicates available entry
	}
	maxi = 0;       // max index into clients[] array

	while (1) {
		nready = poll(clients, maxi + 1, INFTIM);

		if (nready <= 0) {
			continue;
		} 

		// Check new connection
		if (clients[0].revents & POLLRDNORM) {
			clilen = sizeof(cliaddr);
			if ((connfd = accept(listen_sock, (struct sockaddr *)&cliaddr, &clilen)) < 0) {
				fprintf(stderr, "Error: accept\n");
				return;
			}

			printf("Accept socket %d (%s : %hu)\n", 
			   connfd, 
			   inet_ntoa(cliaddr.sin_addr),
			   ntohs(cliaddr.sin_port));

			// Save client socket into clients array
			for (i = 0; i < OPEN_MAX; i++) {
				if (clients[i].fd < 0) {
					clients[i].fd = connfd;
					break;
				}
			}

			// No enough space in clients array
			if (i == OPEN_MAX) {
				fprintf(stderr, "Error: too many clients\n");
				close(connfd);
			}

			clients[i].events = POLLRDNORM;

			if (i > maxi) {
				maxi = i;
			}

			// No more readable file descriptors
			if (--nready <= 0) {
				continue;
			}
		} 

		// Check all clients to read data
		for (i = 1; i <= maxi; i++) {
			if ((sockfd = clients[i].fd) < 0) {
				continue;
			}

			// If the client is readable or errors occur
			if (clients[i].revents & (POLLRDNORM | POLLERR)) {
				server_echo(sockfd, &clients[i].fd);

				// No more readable file descriptors
				if (--nready <= 0) {
					break;
				}
			}
		}
	}
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
	// printf("Server started."); ???
	run_server(listen_sock);
	cleanup_server(listen_sock);
	
	return 0;
}