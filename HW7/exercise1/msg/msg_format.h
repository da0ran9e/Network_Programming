#ifndef MSG_FORMAT_H
#define MSG_FORMAT_H

#include <stdint.h>
#define BUFF_MAX 1000

typedef struct msg_format {
	char opcode;		// 1 byte
	uint16_t length;	// 2 bytes
	char payload[BUFF_MAX + 1];
} Msg;

int sendMsg(int sockfd, Msg * msg);
int recvMsg(int sockfd, Msg * msg);

#endif