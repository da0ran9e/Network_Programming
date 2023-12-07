#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdint.h>
#define BUFFER 1024

typedef struct ServerMessage {
	char opcode; // 1 byte		
	int length;	// 2 bytes
	char payload[BUFFER - 3];
} ServerMessage;

#define SERVER_MESSAGE(str) \
    {                       \
        .opcode = (str)[0], \
        .length = *(int *)((str) + 1), \
        .payload = {0}, \
    }

int recvMessage(int sockfd, ServerMessage * msg);
int sendMessage(int sockfd, ServerMessage * msg);

#endif