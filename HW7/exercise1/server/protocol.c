#include "include/protocol.h"

char buffer[BUFFER];

int recvMessage(int sockfd, ServerMessage * msg) {
    memset(buffer, 0, sizeof(buffer));

    int received = recv(sockfd, buffer, BUFF_MAX + EXTRA_SIZE, 0);
    if (received < 0) {
        perror("\nError: ");
        return -1;
    } else if (received == 0) {
        printf("Connection closed.\n");
        return -1;
    }

    *msg = SERVER_MESSAGE(buffer);
    return received;
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

