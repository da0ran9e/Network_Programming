#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h> // get file size

#include "msg_format.h"
#include "edit_file.h"
#include "debug.h"

#include "tcp_file.h"


/*
* Send file to socket
* [IN] sockfd: what socket to use
* [IN] filename: name of file to send (maybe path)
* [OUT] total_bytes_sent: Total sent
*/

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


/*
* Receive file from socket
* [IN] sockfd: what socket to use
* [IN] filename: name of file to write to (maybe path)
*/
void recv_file_from_socket(int sockfd, char * filename, int editmode, int key) {
	int bytes_received;
	Msg newMsg;

	// Receive actual file
	DEBUG_PRINTF("Writing to '%s'.\n", filename);
	FILE * recv_file = fopen(filename, "ab");
	if (!recv_file) {
		perror("\nRecv file error: ");
		return;
	}


	while(1) {
		bytes_received = recvMsg(sockfd, &newMsg);
		if (bytes_received < 0) {
			// ERROR HERE
			break;
		}

		if (newMsg.length == 0) {	// Final message
			DEBUG_PRINTF("-- EOF EOF EOF --\n");
			break;
		}

		edit_file(newMsg.payload, editmode, key);
		fwrite(newMsg.payload, 1, newMsg.length, recv_file);
	}

	fclose(recv_file);
}