#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <ctype.h>
#include <inttypes.h>

#define SERVER_ADDR "127.0.0.1"
#define CHOSEN_PORT 5501
#define BACKLOG 20
#define BUFF_SIZE 1024
#define NORMAL_MODE 0
#define DELETE_MODE 1

/* Handler process signal */
void sig_chld(int signo);

int initialize_client(int port);
void client_echo(int client_sock);
void send_file_to_socket(int sockfd, char *filename, int mode);
void recv_file_from_socket(int sockfd, char *filename);
void cleanup_client(int client_sock);
void edit_buffer(char *str);

int main() {
    int client_sock;

    client_sock = initialize_client(CHOSEN_PORT);

    run_client(client_sock);
    cleanup_client(client_sock);

    return 0;
}

int initialize_client(int port) {
    int client_sock;
    struct sockaddr_in server_addr; /* server's address information */

    // Step 1: Construct socket
    client_sock = socket(AF_INET, SOCK_STREAM, 0);

    // Step 2: Specify server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);

    // Step 3: Request to connect to the server
    if (connect(client_sock, (struct sockaddr *) &server_addr, sizeof(struct sockaddr)) < 0) {
        perror("Error: Unable to connect to server");
        close(client_sock);
        exit(1);
    }

    return client_sock;
}

void client_echo(int client_sock) {
    char buff[BUFF_SIZE + 1];
    int total_bytes_sent = 0;

    // Step 4: Communicate with the server
    printf("\nFile path to send: ");
    memset(buff, '\0', (strlen(buff) + 1));
    fgets(buff, BUFF_SIZE, stdin);

    // Remove all \r\n
    buff[strcspn(buff, "\r\n")] = 0;

    send_file_to_socket(client_sock, buff, NORMAL_MODE);
    recv_file_from_socket(client_sock, buff);

    printf("\nTotal bytes sent: %d\n", total_bytes_sent);
}

void send_file_to_socket(int sockfd, char *filename, int mode) {
    struct stat filestats;
    int status;

    status = stat(filename, &filestats);
    if (status != 0) {
        perror("\nFile Error: ");
        exit(1);
    }

    int bytes_sent, bytes_read;

    // Send file size
    int32_t filesize_to_send = htonl(filestats.st_size);
    bytes_sent = send(sockfd, &filesize_to_send, sizeof(filesize_to_send), 0);
    if (bytes_sent < 0) {
        perror("\nError: ");
    }

    // Send the actual file
    char buff[BUFF_SIZE + 1];
    FILE *send_file = fopen(filename, "rb");

    int total_bytes_sent = 0;

    while ((bytes_read = fread(buff, 1, BUFF_SIZE, send_file)) > 0) {
        buff[bytes_read] = '\0';
        bytes_sent = send(sockfd, buff, bytes_read, 0);
        if (bytes_sent < 0) {
            perror("\nError: ");
        }

        total_bytes_sent += bytes_sent;
        memset(buff, 0, sizeof(buff));
    }

    fclose(send_file);

    if (mode == DELETE_MODE) {
        if (remove(filename) == 0) {
            printf("Deleted temp successfully");
        } else {
            printf("Unable to delete the temp file");
        }
    }
}

void recv_file_from_socket(int sockfd, char *filename) {
    int32_t filesize_to_get = 0;
    int bytes_received;

    bytes_received = recv(sockfd, &filesize_to_get, sizeof(filesize_to_get), 0); // Get file size
    if (bytes_received < 0)
        perror("\nError: ");
    else if (bytes_received == 0) {
        printf("Connection closed.\n");
        return;
    }

    int filesize = ntohl(filesize_to_get);

    // Receive the actual file
    char buff[BUFF_SIZE + 1];
    FILE *recv_file = fopen(filename, "wb");

    while (filesize > 0) {
        bytes_received = recv(sockfd, buff, BUFF_SIZE, 0);
        if (bytes_received < 0)
            perror("\nError: ");
        else if (bytes_received == 0) {
            printf("Connection closed.\n");
            break;
        }
        buff[bytes_received] = '\0';

        edit_buffer(buff);
        fwrite(buff, 1, bytes_received, recv_file);

        filesize -= bytes_received;
        memset(buff, 0, sizeof(buff));
    }

    fclose(recv_file);
}

void cleanup_client(int client_sock) {
    // Step 5: Close the socket
    close(client_sock);
}

void edit_buffer(char *str) {
    do {
        *str = toupper((unsigned char) *str);
    } while (*str++);
}
