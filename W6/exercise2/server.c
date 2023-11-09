#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/wait.h>
#include <errno.h>
#include <arpa/inet.h>
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

int initialize_server(int port);
void run_server(int listen_sock);
void server_echo(int sockfd);
void cleanup_server(int listen_sock);
int send_file_to_socket(int sockfd, char *filename, int mode);
void recv_file_from_socket(int sockfd, char *filename);
void edit_buffer(char *str);

int main() {
    int listen_sock; /* file descriptors */

    listen_sock = initialize_server(CHOSEN_PORT);

    run_server(listen_sock);
    cleanup_server(listen_sock);

    return 0;
}

int initialize_server(int port) {
    int listen_sock; /* file descriptors */
    struct sockaddr_in server; /* server's address information */

    if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket() error");
        exit(1);
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = htonl(INADDR_ANY); /* INADDR_ANY puts your IP address automatically */

    if (bind(listen_sock, (struct sockaddr *) &server, sizeof(server)) == -1) {
        perror("Error: ");
        close(listen_sock);
        exit(1);
    }

    if (listen(listen_sock, BACKLOG) == -1) {
        perror("Error: ");
        close(listen_sock);
        exit(1);
    }

    return listen_sock;
}

void run_server(int listen_sock) {
    /* Establish a signal handler to catch SIGCHLD */
    signal(SIGCHLD, sig_chld);

    struct sockaddr_in client_addr; /* client's address information */
    socklen_t sin_size;

    int conn_sock;
    pid_t pid;

    while (1) {
        sin_size = sizeof(struct sockaddr_in);
        if ((conn_sock = accept(listen_sock, (struct sockaddr *) &client_addr, &sin_size)) == -1) {
            if (errno == EINTR)
                continue;
            else {
                perror("Error: ");
                return;
            }
        }

        /* For each client, fork spawns a child, and the child handles the new client */
        pid = fork();

        if (pid == 0) {
            close(listen_sock);
            printf("You got a connection from %s\n", inet_ntoa(client_addr.sin_addr)); /* prints client's IP */
            server_echo(conn_sock);
            close(conn_sock);
            exit(0); // Exit child
        }

        /* The parent closes the connected socket since the child handles the new client */
        close(conn_sock);
    }
}

void edit_buffer(char *str) {
    do {
        *str = toupper((unsigned char) *str);
    } while (*str++);
}

int send_file_to_socket(int sockfd, char *filename, int mode) {
    struct stat filestats;
    int status;

    status = stat(filename, &filestats);
    if (status != 0) {
        perror("File Error: ");
        exit(1);
    }

    int bytes_sent, bytes_read;

    // Send file size
    int32_t filesize_to_send = htonl(filestats.st_size);
    bytes_sent = send(sockfd, &filesize_to_send, sizeof(filesize_to_send), 0);
    if (bytes_sent < 0) {
        perror("Error: ");
    }

    // Send actual file
    char buff[BUFF_SIZE + 1];
    FILE *send_file = fopen(filename, "rb");

    int total_bytes_sent = 0;

    while ((bytes_read = fread(buff, 1, BUFF_SIZE, send_file)) > 0) {
        buff[bytes_read] = '\0';
        bytes_sent = send(sockfd, buff, bytes_read, 0);
        if (bytes_sent < 0) {
            perror("Error: ");
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

    return total_bytes_sent;
}

void recv_file_from_socket(int sockfd, char *filename) {
    int32_t filesize_to_get = 0;
    int bytes_received;

    bytes_received = recv(sockfd, &filesize_to_get, sizeof(filesize_to_get), 0); // Get file size
    if (bytes_received < 0)
        perror("Error: ");
    else if (bytes_received == 0) {
        printf("Connection closed.\n");
        return;
    }

    int filesize = ntohl(filesize_to_get);

    // Receive actual file
    char buff[BUFF_SIZE + 1];
    FILE *recv_file = fopen(filename, "wb");

    while (filesize > 0) {
        bytes_received = recv(sockfd, buff, BUFF_SIZE, 0);
        if (bytes_received < 0)
            perror("Error: ");
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

void server_echo(int sockfd) {
    char tempfilename[] = ".~temp.txt";

    while (1) {
        recv_file_from_socket(sockfd, tempfilename);
        send_file_to_socket(sockfd, tempfilename, DELETE_MODE);
    }
}

void cleanup_server(int listen_sock) {
    close(listen_sock);
}

void sig_chld(int signo) {
    pid_t pid;
    int stat;

    /* Wait for the child process to terminate */
    while ((pid = waitpid(-1, &stat, WNOHANG)) > 0)
        printf("Child %d terminated\n", pid);
}
