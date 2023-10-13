#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_BUFF_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s PortNumber\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]); //string to integer

    int sockfd;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    // Create a socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation error: ");
        return 0;
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(port);

    // Bind the socket to the specified port
    if (bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Bind error");
        return 0;
    }

    printf("Server is running at port %d.\n", port);

    char buffer[MAX_BUFF_SIZE];
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&clientAddr, &clientAddrLen);

        if (n < 0) {
            perror("Receive error");
            continue;
        }

        char alphabetStr[MAX_BUFF_SIZE];
        char digitStr[MAX_BUFF_SIZE];
        char errorStr[] = "Error: Input contains non-alphanumeric characters.";

        int alphaIndex = 0;
        int digitIndex = 0;
        int hasError = 0;

        for (int i = 0; i < n; i++) {
            if (isalpha(buffer[i])) {
                alphabetStr[alphaIndex++] = buffer[i];
            } else if (isdigit(buffer[i])) {
                digitStr[digitIndex++] = buffer[i];
            } else {
                hasError = 1;
                break;
            }
        }

        if (hasError) {
            sendto(sockfd, errorStr, strlen(errorStr), 0, (struct sockaddr *)&clientAddr, clientAddrLen);
        } else {
            alphabetStr[alphaIndex] = '\0';
            digitStr[digitIndex] = '\0';

            sendto(sockfd, alphabetStr, strlen(alphabetStr), 0, (struct sockaddr *)&clientAddr, clientAddrLen);
            sendto(sockfd, digitStr, strlen(digitStr), 0, (struct sockaddr *)&clientAddr, clientAddrLen);
        }
    }

    close(sockfd);
    return 0;
}
