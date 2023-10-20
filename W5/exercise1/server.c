#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAXLINE 1024

void processString(char *input, char *alphaStr, char *digitStr) {
    int alphaIdx = 0;
    int digitIdx = 0;

    for (int i = 0; input[i] != '\0'; i++) {
        if (isalpha(input[i])) {
            alphaStr[alphaIdx++] = input[i];
        } else if (isdigit(input[i])) {
            digitStr[digitIdx++] = input[i];
        } else {
            // Handle error if non-alphabet and non-digit character is found
            printf("Error: Non-alphabet and non-digit character found\n");
            return;
        }
    }
    alphaStr[alphaIdx] = '\0';
    digitStr[digitIdx] = '\0';
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s Port_Number\n", argv[0]);
        exit(1);
    }

    int listenfd, connfd;
    char buffer[MAXLINE];
    char alphaStr[MAXLINE];
    char digitStr[MAXLINE];
    struct sockaddr_in servaddr, client_addr;
    socklen_t client_len;

    int port = atoi(argv[1]);

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        perror("bind");
        exit(1);
    }

    if (listen(listenfd, 5) == -1) {
        perror("listen");
        exit(1);
    }

    printf("Server listening on port %d...\n", port);

    client_len = sizeof(client_addr);
    connfd = accept(listenfd, (struct sockaddr *)&client_addr, &client_len);

    while (1) {
        ssize_t n = recv(connfd, buffer, MAXLINE, 0);
        if (n <= 0) {
            perror("recv");
            break;
        }

        buffer[n] = '\0';

        processString(buffer, alphaStr, digitStr);

        if (strlen(alphaStr) > 0 || strlen(digitStr) > 0) {
            send(connfd, alphaStr, strlen(alphaStr), 0);
            send(connfd, digitStr, strlen(digitStr), 0);
        }
    }

    close(connfd);
    close(listenfd);

    return 0;
}
