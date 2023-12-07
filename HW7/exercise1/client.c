#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h> 
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>

#include "client/include/protocol.h"
#include "client/include/file_handler.h"

#define BACKLOG 20
#define MAX_BUFFER_SIZE 5120

void client_echo(int client_sock) {
	char filename[200];

	printf("file name: ");
	fgets(filename, 200, stdin);
	filename[strcspn(filename, "\r\n")] = 0;		// Remove all \r\n

	int opcode, key;
	printf("Send mode (0: Encode, 1: Decode): ");
	scanf(" %d", &opcode);

	printf("Key (int): ");
	scanf(" %d", &key);
	
	ServerMessage newMsg;
	newMsg.opcode = opcode % 2;
	sprintf(newMsg.payload, "%d", key);
	newMsg.length = strlen(newMsg.payload) + 1;

	sendMessage(client_sock, &newMsg); // Send msg first

	// Send file
	send_file_to_socket(client_sock, filename);

	// Delete temporary file
	delete_file(filename);

	recv_file_from_socket(client_sock, filename, -1, 0);
	printf("Received file '%s'\n", filename);
}

int sendMessage(int sockfd, ServerMessage * msg) {
	memset(buffer, 0, sizeof(buffer));

    memcpy(buffer, &msg.opcode, 1); 
    memcpy(buffer + 1, &msg.length, 2); 
    memcpy(buffer + 3, msg.payload, BUFFER - 3);

	int bytes_sent = send(sockfd, buffer, msg->length + EXTRA_SIZE, 0);
	if(bytes_sent < 0) {
		perror("\nError: ");
		return -1;
	}

	return bytes_sent;
}

void recv_file_from_socket(int sockfd, char * filename, int editmode, int key) {
	int bytes_received;
	ServerMessage newMsg;

	FILE * recv_file = fopen(filename, "ab");
	if (!recv_file) {
		perror("\nReceiving error: ");
		return;
	}


	while(1) {
		bytes_received = recvMessage(sockfd, &newMsg);
		edit_file(newMsg.payload, editmode, key);
		fwrite(newMsg.payload, 1, newMsg.length, recv_file);
	}

	fclose(recv_file);
}

void send_file_to_socket(int sockfd, char * filename) {
	struct stat filestats;

	int status = stat(filename, &filestats);
	if (status != 0) {
		perror("\nFile Error: ");
		exit(1);
	}


	// Starting
	int bytes_sent, bytes_read;

	// Send actual file
	Msg newMsg;
	newMsg.opcode = 2;	// Send, recv file

	int total_bytes_sent = 0;
	FILE * send_file = fopen(filename, "rb");

	while((bytes_read = fread(newMsg.payload, 1, BUFF_MAX, send_file)) > 0) {
		newMsg.length = bytes_read;
		newMsg.payload[bytes_read] = '\0';

		bytes_sent = sendMsg(sockfd, &newMsg);
		if (bytes_sent < 0) {
			// ERROR HERE
			continue;
		}

		total_bytes_sent += bytes_sent;
	}

	fclose(send_file);

	newMsg.length = 0;
	memset(newMsg.payload, 0, BUFF_MAX);
	bytes_sent = sendMsg(sockfd, &newMsg);	// Final message
	if (bytes_sent < 0) {
		// ERROR HERE
		// continue;
	}

	DEBUG_PRINTF("Total bytes: %d\n", total_bytes_sent);
}

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
	client_echo(client_sock);
}

void cleanup_client(int client_sock) {
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
