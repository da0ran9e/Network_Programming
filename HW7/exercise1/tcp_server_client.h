#ifndef TCP_SERVER_CLIENT_H
#define TCP_SERVER_CLIENT_H

// EXERCISE 1
#define BACKLOG 20

void server_echo(int sockfd);
void client_echo(int sockfd);

#endif