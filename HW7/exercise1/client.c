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

void clientEcho(int clientSock) {
	char filename[200];

	printf("File name: ");
	fgets(filename, 200, stdin);
	filename[strcspn(filename, "\r\n")] = 0;  

	int opcode, key;
	printf("Options:\n0: Encode\n1: Decode\nChoose an option:");
	scanf(" %d", &opcode);

	printf("Key (int): ");
	scanf(" %d", &key);

	ServerMessage newMsg;
	newMsg.opcode = opcode % 2;
	sprintf(newMsg.payload, "%d", key);
	newMsg.length = strlen(newMsg.payload) + 1;

	sendMessage(clientSock, &newMsg); // Send message first

	// Send file
	sendFileToSocket(clientSock, filename);

	// Delete temporary file
	deleteFile(filename);

	receiveFileFromSocket(clientSock, filename, -1, 0);
	printf("Received file '%s'\n", filename);
}

int sendMessage(int sockfd, ServerMessage *msg) {
	memset(buffer, 0, sizeof(buffer));

	memcpy(buffer, &msg->opcode, 1);
	memcpy(buffer + 1, &msg->length, 2);
	memcpy(buffer + 3, msg->payload, BUFFER - 3);

	int bytesSent = send(sockfd, buffer, msg->length + EXTRA_SIZE, 0);
	if (bytesSent < 0) {
		perror("\nError: ");
		return -1;
	}

	return bytesSent;
}

void receiveFileFromSocket(int sockfd, char *filename, int editMode, int key) {
	int bytesReceived;
	ServerMessage newMsg;

	FILE *recvFile = fopen(filename, "ab");
	if (!recvFile) {
		perror("\nReceiving error: ");
		return;
	}

	while (1) {
		bytesReceived = recvMessage(sockfd, &newMsg);
		editFile(newMsg.payload, editMode, key);
		fwrite(newMsg.payload, 1, newMsg.length, recvFile);
	}

	fclose(recvFile);
}

void sendFileToSocket(int sockfd, char *filename) {
	struct stat fileStats;

	int status = stat(filename, &fileStats);
	if (status != 0) {
		perror("\nFile Error: ");
		exit(1);
	}

	// Starting
	int bytesSent, bytesRead;

	// Send actual file
	ServerMessage newMsg;
	newMsg.opcode = 2;	// Send, receive file

	int totalBytesSent = 0;
	FILE *sendFile = fopen(filename, "rb");

	while ((bytesRead = fread(newMsg.payload, 1, BUFF_MAX, sendFile)) > 0) {
		newMsg.length = bytesRead;
		newMsg.payload[bytesRead] = '\0';

		bytesSent = sendMsg(sockfd, &newMsg);
		if (bytesSent < 0) {
			// ERROR HERE
			continue;
		}

		totalBytesSent += bytesSent;
	}

	fclose(sendFile);

	newMsg.length = 0;
	memset(newMsg.payload, 0, BUFF_MAX);
	bytesSent = sendMsg(sockfd, &newMsg);	// Final message
	if (bytesSent < 0) {
		// ERROR HERE
		// continue;
	}

	DEBUG_PRINTF("Total bytes: %d\n", totalBytesSent);
}

int initializeClient(int port, char *inputAddr) {
	int clientSock;
	struct sockaddr_in serverAddr; /* server's address information */

	// Step 1: Construct socket
	clientSock = socket(AF_INET, SOCK_STREAM, 0);

	// Step 2: Specify server address
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.s_addr = inet_addr(inputAddr);

	// Step 3: Request to connect to server
	if (connect(clientSock, (struct sockaddr *)&serverAddr, sizeof(struct sockaddr)) < 0) {
		printf("Error! Cannot connect to server! Client exit immediately!\n");
		close(clientSock);
		exit(0);
	}

	return clientSock;
}

void runClient(int clientSock) {
	clientEcho(clientSock);
}

void cleanupClient(int clientSock) {
	close(clientSock);
}

int main(int argc, char *argv[]) {
	if (argc != 3) {
		printf("Usage: %s IPAddress PortNumber\n", argv[0]);
		exit(1);
	}

	int clientSock;

	clientSock = initializeClient(atoi(argv[2]), argv[1]);

	runClient(clientSock);
	cleanupClient(clientSock);

	return 0;
}
