#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_BUFF_SIZE 1024

void sendMessageToServer(int sockfd, const struct sockaddr_in *serverAddr) {
    char buffer[MAX_BUFF_SIZE];

    while (1) {
        printf("Enter a string (or '***' to exit): ");
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = '\0';

        if (strcmp(buffer, "***") == 0 || strcmp(buffer, "") == 0) {
            break;
        }

        sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)serverAddr, sizeof(*serverAddr));
    }
}

void receiveAndPrintResults(int sockfd) {
    char buffer[MAX_BUFF_SIZE];

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int n = recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, NULL);

        if (n < 0) {
            perror("Receive error");
            break;
        }

        printf("Received: %s\n", buffer);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s IPAddress PortNumber\n", argv[0]);
        return 1;
    }

    char *serverIP = argv[1];
    int port = atoi(argv[2]);

    int sockfd;
    struct sockaddr_in serverAddr;

    // Create a socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation error");
        return 1;
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(serverIP); //bind to ip address
    serverAddr.sin_port = htons(port); //bind to port

    sendMessageToServer(sockfd, &serverAddr);

    receiveAndPrintResults(sockfd);

    // char buffer[MAX_BUFF_SIZE];

    // while (1) {
    //     printf("Enter a string (or '***' to exit): ");
    //     fgets(buffer, sizeof(buffer), stdin);
    //     buffer[strcspn(buffer, "\n")] = '\0';

    //     if (strcmp(buffer, "***") == 0 || strcmp(buffer, "") == 0) {
    //         break;
    //     }

    //     sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

    //     memset(buffer, 0, sizeof(buffer));
    //     int n = recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, NULL);

    //     if (n < 0) {
    //         perror("Receive error");
    //         break;
    //     }

    //     printf("Received alphabet string: %s\n", buffer);

    //     memset(buffer, 0, sizeof(buffer));
    //     n = recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, NULL);

    //     if (n < 0) {
    //         perror("Receive error");
    //         break;
    //     }

    //     printf("Received digit string: %s\n", buffer);
    // }
    close(sockfd);
    return 0;
}
