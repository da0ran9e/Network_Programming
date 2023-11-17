#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFF_SIZE 1024
#define STATUS_WRONG_CREDENTIAL 0
#define STATUS_NO_ACCOUNT 1
#define STATUS_LOGGED_IN 2
#define STATUS_LOCKED 3

int initialize_client(int port, const char *inputAddr);
void run_client(int client_sock);
void cleanup_client(int client_sock);
void client_echo(int client_sock);

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s IPAddress PortNumber\n", argv[0]);
        exit(1);
    }

    int client_sock;

    client_sock = initialize_client(atoi(argv[2]), argv[1]);

    run_client(client_sock);
    cleanup_client(client_sock);

    return 0;
}

int initialize_client(int port, const char *inputAddr) {
    int client_sock;
    struct sockaddr_in server_addr;

    // Step 1: Construct socket
    client_sock = socket(AF_INET, SOCK_STREAM, 0);

    // Step 2: Specify server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(inputAddr);

    // Step 3: Request to connect server
    if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) < 0) {
        printf("Error! Cannot connect to server! Client exits immediately!\n");
        close(client_sock);
        exit(EXIT_FAILURE);
    }

    return client_sock;
}

void run_client(int client_sock) {
    printf("Client started.\n");
    client_echo(client_sock);
}

void cleanup_client(int client_sock) {
    // Step 5: Close socket
    close(client_sock);
}

void client_echo(int client_sock) {
    char buff[BUFF_SIZE + 1];
    int bytes_sent, bytes_received, msg_len;

    char username[BUFF_SIZE / 2 - 1];
    char password[BUFF_SIZE / 2 - 1];

    while (1) {
        printf("\n[SIGN IN]\n");
        printf("Username: ");
        if (scanf(" %s", username) != 1) {
            printf("Invalid input. Exiting.\n");
            break;
        }
        printf("Password: ");
        if (scanf(" %s", password) != 1) {
            printf("Invalid input. Exiting.\n");
            break;
        }

        memset(buff, '\0', BUFF_SIZE);
        sprintf(buff, "%s %s", username, password);

        msg_len = strlen(buff);
        bytes_sent = send(client_sock, buff, msg_len, 0);
        if (bytes_sent < 0)
            perror("\nError: ");

        bytes_received = recv(client_sock, buff, BUFF_SIZE, 0);
        if (bytes_received < 0)
            perror("\nError: ");
        else if (bytes_received == 0) {
            printf("Echo: Connection closed.\n");
            break;
        }
        buff[bytes_received] = '\0';

        int account_status = -1;
        sscanf(buff, " %d", &account_status);

        switch (account_status) {
        case STATUS_WRONG_CREDENTIAL:
            printf("\nPassword is incorrect.\n");
            break;
        case STATUS_NO_ACCOUNT:
            printf("\nUsername doesn't exist.\n");
            break;
        case STATUS_LOGGED_IN:
            printf("\nLogged in. Congratulations!\n");
            return;
        case STATUS_LOCKED:
            printf("\nAccount locked. (5 or more failed login attempts)\n");
            return;
        default:
            perror("\nSOMETHING WRONG HAPPENED? TERMINATING.\n");
            break;
        }
    }
}
