#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h> 
#include <string.h>

// #include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "msg/msg_format.h"	// Msg
#include "msg/tcp_file.h"	// send, recv
#include "msg/edit_file.h"	// delete

#include "tcp_server_client.h"

#include <sys/select.h> 

void handle_msg(int sockfd, Msg * msg);

void server_echo(int listen_sock) {
	if (create_server_directory() < 0) {
		return;
	}
	int maxi, maxfd;
	int client[FD_SETSIZE];
	fd_set	readfds, allset;

	maxfd = listen_sock;
	maxi = -1;					/* index into client[] array */
	for (int i = 0; i < FD_SETSIZE; i++)
		client[i] = -1;			/* -1 indicates available entry */
	FD_ZERO(&allset);
	FD_SET(listen_sock, &allset);


	//Step 4: Communicate with clients
	socklen_t clilen;
	struct sockaddr_in cliaddr;

	int connfd, sockfd;
	int nready;
	while (1) {
		readfds = allset;		/* structure assignment */
		nready = select(maxfd+1, &readfds, NULL, NULL, NULL);
		if(nready < 0) {
			perror("\nError: ");
			return;
		}
		
		if (FD_ISSET(listen_sock, &readfds)) {	/* new client connection */
			clilen = sizeof(cliaddr);
			if((connfd = accept(listen_sock, (struct sockaddr *) &cliaddr, &clilen)) < 0) {
				perror("\nError: ");
			} else {
				printf("You got a connection from %s\n", inet_ntoa(cliaddr.sin_addr)); /* prints client's IP */
				int count;
				for (count = 0; count < FD_SETSIZE; count++)
					if (client[count] < 0) {
						client[count] = connfd;	/* save descriptor */
						break;
					}
				if (count == FD_SETSIZE){
					printf("\nToo many clients");
					close(connfd);
				}

				FD_SET(connfd, &allset);	/* add new descriptor to set */
				if (connfd > maxfd) {
					maxfd = connfd;		/* for select */
				}
				
				if (count > maxi) {
					maxi = count;		/* max index in client[] array */
				}

				if (--nready <= 0) continue;		/* no more readable descriptors */
			}
		}

		Msg newMsg;
		for (int i = 0; i <= maxi; i++) {	/* check all clients for data */
			if ( (sockfd = client[i]) < 0) {
				continue;
			}

			if (FD_ISSET(sockfd, &readfds)) {
				ssize_t bytes_received = recvMsg(sockfd, &newMsg);
				if (bytes_received <= 0) {
					FD_CLR(sockfd, &allset);
					close(sockfd);
					client[i] = -1;
				} else {
					handle_msg(sockfd, &newMsg);
				}

				if (--nready <= 0) break;		/* no more readable descriptors */
			}
		}
	}
}

extern const char directoryName[];

void handle_msg(int sockfd, Msg * msg) {
	int key = atoi(msg->payload);
	int editmode = msg->opcode;
	char filename[100] = "";
	strcat(filename, directoryName);
	strcat(filename, "/temp");
	strcat(filename, msg->payload);
	strcat(filename, ".txt");

	// Receive and encode/decode file
	recv_file_from_socket(sockfd, filename, editmode, key);

	// Send file back
	send_file_to_socket(sockfd, filename);

	// Delete temporary file
	delete_file(filename);
}

void client_echo(int client_sock) {
	char filename[1024];

	printf("\nFile name to send: ");
	fgets(filename, 1024, stdin);
	filename[strcspn(filename, "\r\n")] = 0;		// Remove all \r\n

	int opcode, key;
	printf("Send mode (0: Encode, 1: Decode): ");
	scanf(" %d", &opcode);

	printf("Key (int): ");
	scanf(" %d", &key);
	
	Msg newMsg;
	newMsg.opcode = opcode % 2;
	sprintf(newMsg.payload, "%d", key);
	newMsg.length = strlen(newMsg.payload) + 1;

	sendMsg(client_sock, &newMsg); // Send msg first

	// Send file
	send_file_to_socket(client_sock, filename);

	// Delete temporary file
	delete_file(filename);

	recv_file_from_socket(client_sock, filename, -1, 0);
	printf("Received file '%s'\n", filename);
}