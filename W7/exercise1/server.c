#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <ctype.h>  // Include ctype.h for toupper function

#define BUFF_SIZE 1024
#define BACKLOG 10
#define THREAD_MAX 5
#define CHOSEN_PORT 12345

void *handle_client_connection(void *client_socket);
int initialize_server(int port);
void start_server(int listen_sock);
void echo_to_client(int sockfd);
void cleanup_server(int listen_sock);

int main() {
    int listen_sock;

    listen_sock = initialize_server(CHOSEN_PORT);
    start_server(listen_sock);
    cleanup_server(listen_sock);

    return 0;
}

void *handle_client_connection(void *client_socket) {
    int conn_sock = *(int *)client_socket;
    echo_to_client(conn_sock);
    close(conn_sock);

    return NULL;
}

int initialize_server(int port) {
    int listen_sock;
    struct sockaddr_in server;

    if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket() error");
        exit(EXIT_FAILURE);
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listen_sock, (struct sockaddr *)&server, sizeof(server)) == -1) {
        perror("\nError: ");
        close(listen_sock);
        exit(EXIT_FAILURE);
    }

    if (listen(listen_sock, BACKLOG) == -1) {
        perror("\nError: ");
        close(listen_sock);
        exit(EXIT_FAILURE);
    }

    printf("Server created.\n\n");
    return listen_sock;
}

void start_server(int listen_sock) {
    struct sockaddr_in client_addr;
    socklen_t sin_size = sizeof(struct sockaddr_in);
    int conn_sock;

    int num_threads = 0;
    pthread_t threads[THREAD_MAX];

    while (num_threads < THREAD_MAX) {
        printf("Listening...\n");
        conn_sock = accept(listen_sock, (struct sockaddr *)&client_addr, &sin_size);

        if (pthread_create(&threads[num_threads], NULL, handle_client_connection, &conn_sock) < 0) {
            perror("Could not create thread");
            exit(EXIT_FAILURE);
        }

        if (conn_sock < 0) {
            printf("Server accept failed...\n");
            exit(EXIT_FAILURE);
        } else {
            printf("Server accepted a client from %s\n", inet_ntoa(client_addr.sin_addr));
        }

        printf("Handler assigned.\n");
        num_threads++;
    }

    for (int k = 0; k < THREAD_MAX; k++) {
        pthread_join(threads[k], NULL);
        close(conn_sock);  // This should be threads[k] instead of conn_sock
    }
}

void echo_to_client(int sockfd) {
    char buff[BUFF_SIZE + 1];
    int bytes_sent, bytes_received;

    while (1) {
        bytes_received = recv(sockfd, buff, BUFF_SIZE, 0);

        if (bytes_received < 0)
            perror("\nError: ");
        else if (bytes_received == 0) {
            printf("Echo: Connection closed.\n");
            break;
        }

        buff[bytes_received] = '\0';

        // Your code here
        if (strlen(buff) == 1 && toupper(buff[0]) == 'Q') {
            break;
        }

        char *ch = buff;
        while (*ch != '\0') {
            *ch = toupper(*ch);
            ch++;
        }
        // End of your code

        bytes_sent = send(sockfd, buff, bytes_received, 0);

        if (bytes_sent < 0)
            perror("\nError: ");
    }
}

void cleanup_server(int listen_sock) {
    close(listen_sock);
}
