#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define MAXLINE 1024

// Function to process the received string
void processString(char* input, char* alphabet, char* digits) {
    int alphaIndex = 0;
    int digitIndex = 0;

    for (int i = 0; input[i] != '\0'; i++) {
        if (isalpha(input[i])) {
            alphabet[alphaIndex++] = input[i];
        } else if (isdigit(input[i])) {
            digits[digitIndex++] = input[i];
        } else {
            // Send an error message if a non-alphabet and non-digit character is found
            strcpy(alphabet, "Error");
            strcpy(digits, "Error");
            return;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s Port_Number\n", argv[0]);
        exit(1);
    }

    int port = atoi(argv[1]);

    int listenfd, connfd;
    char buffer[MAXLINE];
    struct sockaddr_in servaddr;

    // Create socket
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

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

    printf("Server listening on port %d...\n", port);

    while (1) {
        socklen_t client_len = sizeof(servaddr);
        connfd = accept(listenfd, (struct sockaddr *)&servaddr, &client_len);

        int n = recv(connfd, buffer, MAXLINE, 0);
        if (n <= 0) {
            perror("recv");
            break;
        }

        buffer[n] = '\0';
        printf("Received from client: %s", buffer);

        char alphabet[MAXLINE];
        char digits[MAXLINE];

        processString(buffer, alphabet, digits);

        send(connfd, alphabet, strlen(alphabet), 0);
        send(connfd, digits, strlen(digits), 0);

        close(connfd);
    }

    close(listenfd);

    return 0;
}
