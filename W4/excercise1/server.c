#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX_BUF_SIZE 1024

void processString(char *input, char *alphabets, char *digits) {
    int alphaIndex = 0;
    int digitIndex = 0;

    for (int i = 0; input[i] != '\0'; i++) {
        if (isalpha(input[i])) {
            alphabets[alphaIndex++] = input[i];
        } else if (isdigit(input[i])) {
            digits[digitIndex++] = input[i];
        } else {
            printf("Error: String contains non-alphabet and non-digit characters.\n");
            return;
        }
    }

    alphabets[alphaIndex] = '\0';
    digits[digitIndex] = '\0';
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <PortNumber>\n", argv[0]);
        return 1;
    }

    int serverPort = atoi(argv[1]);

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Error: Failed to create socket");
        return 1;
    }

    struct sockaddr_in serverAddr, clientAddr;
    socklen_t addrLen = sizeof(clientAddr);

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(serverPort);

    if (bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Error: Bind failed");
        close(sockfd);
        return 1;
    }

    char clientMsg[MAX_BUF_SIZE];
    char alphabets[MAX_BUF_SIZE];
    char digits[MAX_BUF_SIZE];
    int receivedBytes;

    while (1) {
        receivedBytes = recvfrom(sockfd, clientMsg, sizeof(clientMsg), 0, (struct sockaddr *)&clientAddr, &addrLen);
        if (receivedBytes < 0) {
            perror("Error: Failed to receive data");
            continue;
        }

        clientMsg[receivedBytes] = '\0';
        if (strcmp(clientMsg, "***") == 0 || strcmp(clientMsg, "") == 0) {
            printf("Server shutting down.\n");
            break;
        }

        processString(clientMsg, alphabets, digits);

        sendto(sockfd, alphabets, strlen(alphabets), 0, (struct sockaddr *)&clientAddr, addrLen);
        sendto(sockfd, digits, strlen(digits), 0, (struct sockaddr *)&clientAddr, addrLen);
    }

    close(sockfd);
    return 0;
}
