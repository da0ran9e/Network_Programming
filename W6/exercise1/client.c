#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#define SERVER_ADDR "127.0.0.1"
#define CHOSEN_PORT 5500
#define BACKLOG 20
#define BUFF_SIZE 1024

int initialize_client(int port);
void run_client(int client_sock);
void cleanup_client(int client_sock);

int main() {
    int client_sock;

    client_sock = initialize_client(CHOSEN_PORT);

    if (client_sock == -1) {
        fprintf(stderr, "Client initialization failed\n");
        return 1;
    }

    run_client(client_sock);
    cleanup_client(client_sock);

    return 0;
}

void client_echo(int client_sock) {
    char buff[BUFF_SIZE + 1];
    int bytes_sent, bytes_received, msg_len;
    int total_bytes_sent = 0;   

    //Step 4: Communicate with server
    while(1) {
    /*** Your code here ***/
        printf("\nInsert string to send: ");
        memset(buff,'\0',(strlen(buff)+1));
        fgets(buff, BUFF_SIZE, stdin);
        // Remove all \r\n
        buff[strcspn(buff, "\r\n")] = 0;
        msg_len = strlen(buff);


        if (msg_len == 1 && toupper(buff[0]) == 'Q') {
            break;
        }
    /**********************/
            
        bytes_sent = send(client_sock, buff, msg_len, 0);
        if(bytes_sent < 0)
            perror("\nError: ");

    /*** Your code here ***/
        total_bytes_sent += bytes_sent;
    /**********************/
        
        //receive echo reply
        bytes_received = recv(client_sock, buff, BUFF_SIZE, 0);
        if (bytes_received < 0)
            perror("\nError: ");
        else if (bytes_received == 0) {
            printf("Connection closed.\n");
            break;
        }
        buff[bytes_received] = '\0';

    /*** Your code here ***/
        printf("Reply from server: %s", buff);
    /**********************/
    }

    printf("\nTotal bytes sent: %d\n", total_bytes_sent);
}

int initialize_client(int port) {
    int client_sock;
    struct sockaddr_in server_addr;

    // Step 1: Construct socket
    client_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (client_sock < 0) {
        perror("Error: Unable to create socket");
        return -1;
    }

    // Step 2: Specify server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);

    // Step 3: Request to connect to the server
    if (connect(client_sock, (struct sockaddr*)&server_addr, sizeof(struct sockaddr)) < 0) {
        perror("Error: Unable to connect to server");
        close(client_sock);
        return -1;
    }

    return client_sock;
}

void run_client(int client_sock) {
    // Add your client logic here
}

void cleanup_client(int client_sock) {
    // Step 5: Close the socket
    close(client_sock);
}
