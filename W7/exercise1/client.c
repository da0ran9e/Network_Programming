#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFF_SIZE 1024
#define CHOSEN_PORT 12345
#define SERVER_ADDR "127.0.0.1" // Replace with your server's IP address

int initialize_client(int port);
void run_client(int client_sock);
void cleanup_client(int client_sock);
void client_echo(int client_sock);

int main() {
    int client_sock;

    client_sock = initialize_client(CHOSEN_PORT);
    run_client(client_sock);
    cleanup_client(client_sock);

    return 0;
}

int initialize_client(int port) {
    int client_sock;
    struct sockaddr_in server_addr;

    // Step 1: Construct socket
    client_sock = socket(AF_INET, SOCK_STREAM, 0);

    // Step 2: Specify server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);

    // Step 3: Request to connect server
    if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) < 0) {
        printf("Error! Can not connect to server! Client exit immediately!\n");
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
    int total_bytes_sent = 0;

    // Step 4: Communicate with server
    while (1) {
        printf("\nInsert string to send: ");
        memset(buff, '\0', (strlen(buff) + 1));
        fgets(buff, BUFF_SIZE, stdin);
        // Remove all \r\n
        buff[strcspn(buff, "\r\n")] = 0;
        msg_len = strlen(buff);

        if (msg_len == 1 && toupper(buff[0]) == 'Q') {
            break;
        }

        bytes_sent = send(client_sock, buff, msg_len, 0);
        if (bytes_sent < 0)
            perror("\nError: ");

        total_bytes_sent += bytes_sent;

        // Receive echo reply
        bytes_received = recv(client_sock, buff, BUFF_SIZE, 0);
        if (bytes_received < 0)
            perror("\nError: ");
        else if (bytes_received == 0) {
            printf("Echo: Connection closed.\n");
            break;
        }
        buff[bytes_received] = '\0';

        printf("Reply from server: %s\n", buff);
    }

    printf("\nTotal bytes sent: %d\n", total_bytes_sent);
}
