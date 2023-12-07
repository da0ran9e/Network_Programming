#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

#include "msg_format.h"
#include "debug.h"

void serializeMsg(char * buffer, Msg * msg) {						// Host short to Network short
	// opcode: 1 bytes
	memcpy(buffer, &msg->opcode, sizeof(char));
	buffer += sizeof(char); // Move buffer pointer

	// length: 2 bytes
	uint16_t conv_length = htons(msg->length);	// Host long to Network long
	memcpy(buffer, &conv_length, sizeof(uint16_t));
	buffer += sizeof(uint16_t); // Move buffer pointer

	// payload: (length) bytes
	memcpy(buffer, msg->payload, msg->length);
}

void deserializeMsg(char * buffer, Msg * msg) {
	// opcode: 1 bytes
	memcpy(&msg->opcode, buffer, sizeof(char));
	buffer += sizeof(char);		// Move buffer pointer

	// length: 2 bytes
	uint16_t conv_length;
	memcpy(&conv_length, buffer, sizeof(uint16_t));	
	buffer += sizeof(uint16_t);	// Move buffer pointer
	msg->length = ntohs(conv_length);

	// payload: (length) bytes
	strncpy(msg->payload, buffer, msg->length);

	DEBUG_PRINTF("LENGTH %d\n", msg->length);

}


void printMsg(Msg * msg) {
	DEBUG_PRINTF("%d:%d:%s\n", msg->opcode, msg->length, msg->payload);
}

// SENDING AND RECEIVING
const int EXTRA_SIZE = sizeof(char) + sizeof(uint16_t) + 1;

char srbuffer[1100];

int sendMsg(int sockfd, Msg * msg) {
	memset(srbuffer, 0, sizeof(srbuffer));
	serializeMsg(srbuffer, msg);

	int bytes_sent = send(sockfd, srbuffer, msg->length + EXTRA_SIZE, 0);
	if(bytes_sent < 0) {
		perror("\nError: ");
		return -1;
	}

	return bytes_sent;
}

int recvMsg(int sockfd, Msg * msg) {
	memset(srbuffer, 0, sizeof(srbuffer));
	// memset(msg->payload, 0, sizeof(msg->payload));

	int bytes_received = recv(sockfd, srbuffer, BUFF_MAX + EXTRA_SIZE, 0);
	if (bytes_received < 0) {
		perror("\nError: ");
		return -1;
	} else if (bytes_received == 0) {
		printf("Connection closed.\n");
		return -1;
	}
	// srbuffer[bytes_received] = '\0';
	deserializeMsg(srbuffer, msg);

	return bytes_received;
}