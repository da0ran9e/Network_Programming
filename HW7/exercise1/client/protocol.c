#include "include/protocol.h"

char buffer[BUFFER];

int sendMsg(int sockfd, Msg * msg) {
	memset(buffer, 0, sizeof(buffer));
	serializeMsg(buffer, msg);

	int bytes_sent = send(sockfd, buffer, msg->length + EXTRA_SIZE, 0);
	if(bytes_sent < 0) {
		perror("\nError: ");
		return -1;
	}

	return bytes_sent;
}