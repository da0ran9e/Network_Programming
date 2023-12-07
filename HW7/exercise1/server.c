#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <sys/select.h> 
#include <dirent.h>

#include "server/include/protocol.h"
#include "server/include/file_handler.h"
#include "server/include/caesar.h"

#define BACKLOG 20

void send_file_to_socket(int sockfd) {
	int sent, offset;
	ServerMessage newMsg;
	newMsg.opcode = 2;

    struct dirent* entry;
    struct stat entryStat;
    time_t lastModifiedTime = 0;
    char* lastModifiedFile = NULL;

	DIR* directory = opendir(folderPath);
    if (directory == NULL) {
        perror("Error opening directory");
        return NULL;
    }

    while ((entry = readdir(directory)) != NULL) {
        char filePath[PATH_MAX];
        snprintf(filePath, sizeof(filePath), "%s/%s", folderPath, entry->d_name);

        if (stat(filePath, &entryStat) == 0) {
            if (S_ISREG(entryStat.st_mode) && entryStat.st_mtime > lastModifiedTime) {
                lastModifiedTime = entryStat.st_mtime;
                if (lastModifiedFile != NULL) {
                    free(lastModifiedFile);
                }
                lastModifiedFile = strdup(entry->d_name);
            }
        }
    }

    closedir(directory);

	FILE * file_data = fopen(lastModifiedFile, "rb");

	while((offset = fread(newMsg.payload, 1, BUFFER, file_data)) > 0) {
		newMsg.length = offset;
		newMsg.payload[offset] = '\0';
	}

	fclose(file_data);

	newMsg.length = 0;
	memset(newMsg.payload, 0, BUFFER);
	sent = sendMessage(sockfd, &newMsg);	
}

char fileBuffer[5120];

void recv_file_from_socket(int sockfd, int editmode, int key) {
	int bytes_received;
	ServerMessage newMsg;

	while(1) {
		bytes_received = recvMessage(sockfd, &newMsg);
		if (bytes_received < 0) {
			break;
		}

		if (newMsg.length == 0) { //endoffile
			break;
		}

		edit(newMsg.payload, editmode, key);
		strcat(fileBuffer, newMsg.payload);  
	}
    save(fileBuffer);
}

void edit(char * content, int editmode, int key) {
	switch (editmode) {
		case 0:
			encrypt(content, key);
			break;
		case 1:
			decrypt(content, key);
			break;
		default:
            printf("ERROR: opcode notfound!\n");
            return;
	}
}

void handle_msg(int sockfd, ServerMessage * msg) {
	int key = atoi(msg->payload);
	int editmode = msg->opcode;

	recv_file_from_socket(sockfd, filename, editmode, key);

	send_file_to_socket(sockfd, filename);

	delete_file(filename);
}

void run(int listening_socket) {
	int maxi, maxfd;
	int client[FD_SETSIZE];
	fd_set	readfds, allset;

	maxfd = listening_socket;
	maxi = -1;					
	for (int i = 0; i < FD_SETSIZE; i++)
		client[i] = -1;			
	FD_ZERO(&allset);
	FD_SET(listening_socket, &allset);
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
		
		if (FD_ISSET(listening_socket, &readfds)) {	/* new client connection */
			clilen = sizeof(cliaddr);
			if((connfd = accept(listening_socket, (struct sockaddr *) &cliaddr, &clilen)) < 0) {
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

		ServerMessage receivedMessage;
		for (int i = 0; i <= maxi; i++) {	/* check all clients for data */
			if ( (sockfd = client[i]) < 0) {
				continue;
			}

			if (FD_ISSET(sockfd, &readfds)) {
				ssize_t bytes_received = recvMessage(sockfd, &receivedMessage);
				if (bytes_received <= 0) {
					FD_CLR(sockfd, &allset);
					close(sockfd);
					client[i] = -1;
				} else {
					handle_msg(sockfd, &receivedMessage);
				}

				if (--nready <= 0) break;		/* no more readable descriptors */
			}
		}
	}
}

int init(int port) {
	int listening_socket;			/* file descriptors */
	struct sockaddr_in server;	/* server's address information */

	if ((listening_socket=socket(AF_INET, SOCK_STREAM, 0)) == -1 ) {	/* calls socket() */
		printf("socket() error\n");
		exit(1);
	}
	
	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = htonl(INADDR_ANY);	/* INADDR_ANY puts your IP address automatically */

	if(bind(listening_socket, (struct sockaddr*)&server, sizeof(server))==-1) { 
		perror("\nError bind: ");
		close(listening_socket);
		exit(1);
	}

	if(listen(listening_socket, BACKLOG) == -1){
		perror("\nError listen: ");
		close(listening_socket);
		exit(1);
	}

	printf("Server created.\n\n");
	return listening_socket;
}


int main(int argc, char *argv[]){
	if (argc != 2) {
		printf("Usage: %s PortNumber\n", argv[0]);
		exit(1);
	}
	
	int listening_socket; /* file descriptors */

	listening_socket = init(atoi(argv[1]));

	run(listening_socket);
	close(listening_socket);
	
	return 0;
}