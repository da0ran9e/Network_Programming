#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAXLINE 1024

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <Port_Number>\n", argv[0]);
        exit(1);
    }

    int listenfd, connfd;
    socklen_t client_len;
    char buffer[MAXLINE];
    struct sockaddr_in servaddr, client_addr;

    int PORT = atoi(argv[1]);

    // Create socket
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    // Bind socket
    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        perror("bind");
        exit(1);
    }

    // Listen for incoming connections
    if (listen(listenfd, 5) == -1) {
        perror("listen");
        exit(1);
    }

    printf("Server listening on port %d...\n", PORT);

    client_len = sizeof(client_addr);
    connfd = accept(listenfd, (struct sockaddr *)&client_addr, &client_len);

    while (1) {
        ssize_t n = recv(connfd, buffer, MAXLINE, 0);
        if (n <= 0) {
            perror("recv");
            break;
        }

        buffer[n] = '\0';
        printf("Received from client: %s\n", buffer);

        // Process the received string and split it
        char alphabet_result[MAXLINE] = "";
        char digit_result[MAXLINE] = "";
        int alpha_count = 0, digit_count = 0;

        for (int i = 0; i < n; i++) {
            if (isalpha(buffer[i])) {
                alphabet_result[alpha_count++] = buffer[i];
            } else if (isdigit(buffer[i])) {
                digit_result[digit_count++] = buffer[i];
            } else {
                // If the received string contains non-alphabet and non-digit characters, send an error notification
                char error_message[] = "Error";
                send(connfd, error_message, strlen(error_message), 0);
                break;
            }
        }

        if (alpha_count > 0) {
            send(connfd, alphabet_result, alpha_count, 0);
        }

        if (digit_count > 0) {
            send(connfd, digit_result, digit_count, 0);
        }
    }

    close(connfd);
    close(listenfd);
    
    return 0;
}
