#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX_BUF_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <IPAddress> <PortNumber>\n", argv[0]);
        return 1;
    }

    int clientPort = atoi(argv[2);

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Error: Failed to create socket");
        return 1;
    }

    struct sockaddr_in serverAddr;
    socklen_t addrLen = sizeof(serverAddr);

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(argv[1]);
    serverAddr.sin_port = htons(clientPort);

    char clientMsg[MAX_BUF_SIZE];
    char serverResponse[MAX_BUF_SIZE];
    int receivedBytes;

    printf("Enter a string (or '***' to exit): ");
    while (1) {
        fgets(clientMsg, sizeof(clientMsg), stdin);
        clientMsg[strcspn(clientMsg, "\n")] = '\0';

        if (sendto(sockfd, clientMsg, strlen(clientMsg), 0, (struct sockaddr *)&serverAddr, addrLen) < 0) {
            perror("Error: Failed to send data");
            break;
        }

        if (strcmp(clientMsg, "***") == 0 || strcmp(clientMsg, "") == 0) {
            printf("Client closing.\n");
            break;
        }

        receivedBytes = recvfrom(sockfd, serverResponse, sizeof(serverResponse), 0, (struct sockaddr *)&serverAddr, &addrLen);
        if (receivedBytes < 0) {
            perror("Error: Failed to receive data");
            break;
        }

        serverResponse[receivedBytes] = '\0';
        printf("Alphabets: %s\n", serverResponse);

        receivedBytes = recvfrom(sockfd, serverResponse, sizeof(serverResponse), 0, (struct sockaddr *)&serverAddr, &addrLen);
        if (receivedBytes < 0) {
            perror("Error: Failed to receive data");
            break;
        }

        serverResponse[receivedBytes] = '\0';
        printf("Digits: %s\n", serverResponse);

        printf("Enter a string (or '***' to exit): ");
    }

    close(sockfd);
    return 0;
}
