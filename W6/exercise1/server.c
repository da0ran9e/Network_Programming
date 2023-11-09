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

#define SERVER_ADDR "127.0.0.1"
#define CHOSEN_PORT 5500
#define BACKLOG 20
#define BUFF_SIZE 1024

/* Handler process signal */
void sig_chld(int signo);

int initialize_server(int port);
void run_server(int listen_sock);
void cleanup_server(int listen_sock);
void handle_client(int conn_sock);

int main() {
    int listen_sock; /* file descriptors */

    listen_sock = initialize_server(CHOSEN_PORT);
    printf("Server started.\n");
    run_server(listen_sock);
    cleanup_server(listen_sock);

    return 0;
}

int initialize_server(int port) {
    int listen_sock; /* file descriptors */
    struct sockaddr_in server; /* server's address information */

    if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Error: Unable to create socket");
        exit(1);
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listen_sock, (struct sockaddr *) &server, sizeof(server)) == -1) {
        perror("Error: Unable to bind socket");
        close(listen_sock);
        exit(1);
    }

    if (listen(listen_sock, BACKLOG) == -1) {
        perror("Error: Unable to listen on socket");
        close(listen_sock);
        exit(1);
    }

    return listen_sock;
}
void server_echo(int sockfd) {
    char buff[BUFF_SIZE + 1];
    int bytes_sent, bytes_received;
    
    while (1) {
        bytes_received = recv(sockfd, buff, BUFF_SIZE, 0); //blocking
        if (bytes_received < 0)
            perror("\nError: ");
        else if (bytes_received == 0) {
            printf("Connection closed.");
            break;
        }
        buff[bytes_received] = '\0';

        /*** Your code here ***/
        if (strlen(buff) == 1 && toupper(buff[0]) == 'Q') {
            break;
        }

        char* ch = buff;
        while(*ch != '\0') {
            *ch = toupper(*ch);
            ch++;
        }
        /**********************/

        bytes_sent = send(sockfd, buff, bytes_received, 0); /* echo to the client */
        if (bytes_sent < 0)
            perror("\nError: ");
    }
}

void run_server(int listen_sock) {
    /* Establish a signal handler to catch SIGCHLD */
    signal(SIGCHLD, sig_chld);

    struct sockaddr_in client_addr; /* client's address information */
    socklen_t sin_size;

    int conn_sock;

    while (1) {
        sin_size = sizeof(struct sockaddr_in);
        if ((conn_sock = accept(listen_sock, (struct sockaddr *) &client_addr, &sin_size) == -1)) {
            if (errno == EINTR)
                continue;
            else {
                perror("Error: Unable to accept connection");
                return;
            }
        }

        handle_client(conn_sock);

        /* The parent closes the connected socket since the child handles the new client */
        close(conn_sock);
    }
}

void cleanup_server(int listen_sock) {
    close(listen_sock);
}

void handle_client(int conn_sock) {
    printf("You got a connection from %s\n", inet_ntoa(client_addr.sin_addr));
    // Add your server logic here
}

void sig_chld(int signo) {
    pid_t pid;
    int stat;

    /* Wait for the child process to terminate */
    while ((pid = waitpid(-1, &stat, WNOHANG)) > 0)
        printf("\nChild %d terminated\n", pid);
}
