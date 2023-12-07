#ifndef TCP_SERVER_CLIENT_H
#define TCP_SERVER_CLIENT_H

// EXERCISE 2
// #define SERVER_ADDR "127.0.0.1"
// #define CHOSEN_PORT 5500
#define BACKLOG 20
#define BUFF_SIZE 1024

// Declarations and definitions for your header file go here
/*
* Receive and echo message to client
* [IN] sockfd: socket descriptor that connects to client 	
*/
void server_echo(int sockfd, int * pollclientfd);
void client_echo(int client_sock);

#endif