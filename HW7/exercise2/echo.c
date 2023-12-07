#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h> 
#include <string.h>
#include <sys/socket.h>

#include "tcp_server_client.h"
#include "read_accounts.h"

#define STATUS_LOCKED 0
#define STATUS_WRONG_CREDENTIAL 1
#define STATUS_LOGGED_IN 2
#define STATUS_NO_ACCOUNT 3

char buff[BUFF_SIZE + 1];
int bytes_sent, bytes_received;

char username[BUFF_SIZE/2 - 1];
char password[BUFF_SIZE/2 - 1];

void server_echo(int sockfd, int * pollclientfd) {
	int accIndex = -1;

	// while (1) {
		bytes_received = recv(sockfd, buff, BUFF_SIZE, 0); // Get username
		if (bytes_received < 0) {
			perror("\nError: ");
			close(sockfd);
			*pollclientfd = -1;
		} else if (bytes_received == 0) {
			printf("Echo: Connection closed.\n");
			close(sockfd);
			*pollclientfd = -1;
			return;
		}
		buff[bytes_received] = '\0';

	/**********************/
	/*** Your code here ***/
		sscanf(buff, "%s %s", username, password);
		int status = -1;

		accIndex = userGetAccountIndex(username); 

		if (accIndex == -1) {
			// Account doesn't exist
			status = STATUS_NO_ACCOUNT;
		} else {
			// Account exist
			if (userCheckPassword(accIndex, password) == 1) {
				status = STATUS_LOGGED_IN;
				userResetLoginCount(accIndex);
			} else {
				// Wrong password
				status = STATUS_WRONG_CREDENTIAL;
				userAddLoginCount(accIndex);
			}

			// Over 5 times
			if(userCheckLocked(accIndex) == 1) {
				status = STATUS_LOCKED;
			}
		}

		memset(buff,'\0', BUFF_SIZE);	// Clear memory
		sprintf(buff, "%d", status);
	/**********************/
	/**********************/

		bytes_sent = send(sockfd, buff, bytes_received, 0); /* echo to the client */
		if (bytes_sent < 0) {
			perror("\nError: ");
		}
	// }
}

void client_echo(int client_sock) {
	//Step 4: Communicate with server
		// + Each client terminal can only login a single account

	while(1) {
	/**********************/
	/*** Your code here ***/
		printf("\n[SIGN IN]");
		printf("\nUsername: ");
		scanf(" %s", username);
		printf("Password: ");
		scanf(" %s", password);

		memset(buff,'\0', BUFF_SIZE);	// Clear memory
		sprintf(buff, "%s %s", username, password);
	// printf("'%s' '%s'\n", username, password); // Testing

		// Remove all \r\n
		// buff[strcspn(buff, "\r\n")] = 0;
	/**********************/
	/**********************/

		bytes_sent = send(client_sock, buff, strlen(buff), 0);
		if(bytes_sent < 0)
			perror("\nError: ");
		
		//receive echo reply
		bytes_received = recv(client_sock, buff, BUFF_SIZE, 0);
		if (bytes_received < 0)
			perror("\nError: ");
		else if (bytes_received == 0) {
			printf("Echo: Connection closed.\n");
			break;
		}
		buff[bytes_received] = '\0';

	/**********************/
	/*** Your code here ***/
		int account_status = -1;
		sscanf(buff, " %d", &account_status);
		
		switch(account_status) {
			case STATUS_WRONG_CREDENTIAL: 
				printf("\nPassword is incorrect.");
				break;
			case STATUS_NO_ACCOUNT:
				printf("\nUsername doesn't exist.");
				break;

			case STATUS_LOGGED_IN: 
				printf("\nLogged in. Congratulations!\n");
				return;
				break;
			case STATUS_LOCKED: 
				printf("\nAccount locked. (5 or more failed login attempts)\n");
				return;
				break;
			default: 
				perror("\nSOMETHING WRONG HAPPENED? TERMINATING.\n");
				break;
		}
	}
}