#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_BUFF_SIZE 1024

// Function to send data to the server
void sendDataToServer(int sockfd, const char *data, const struct sockaddr *serverAddr, socklen_t len) {
    sendto(sockfd, data, strlen(data), 0, serverAddr, len);
}

// Function to receive data from the server
int receiveDataFromServer(int sockfd, char *buffer) {
    memset(buffer, 0, MAX_BUFF_SIZE);
    return recvfrom(sockfd, buffer, MAX_BUFF_SIZE, 0, NULL, NULL);
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

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation error");
        return 1;
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(serverIP);
    serverAddr.sin_port = htons(port);

    char buffer[MAX_BUFF_SIZE];

    while (1) {
        printf("Enter a string (or '***' to exit): ");
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = '\0';

        if (strcmp(buffer, "***") == 0 || strcmp(buffer, "") == 0) {
            break;
        }

        sendDataToServer(sockfd, buffer, (struct sockaddr *)&serverAddr, sizeof(serverAddr));


        if (receiveDataFromServer(sockfd, buffer) < 0) {
            perror("Receive error");
            break;
        }
        printf("%s\n", buffer);
    }

    close(sockfd);
    return 0;
}
