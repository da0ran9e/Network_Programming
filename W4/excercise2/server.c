#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_BUFF_SIZE 1024

// Function to classify input string
void classifyString(const char *input, char *alphabetStr, char *digitStr, char *errorStr) {
    int alphaIndex = 0;
    int digitIndex = 0;
    int hasError = 0;

    for (int i = 0; input[i] != '\0'; i++) {
        if (isalpha(input[i])) {
            alphabetStr[alphaIndex++] = input[i];
        } else if (isdigit(input[i])) {
            digitStr[digitIndex++] = input[i];
        } else {
            hasError = 1;
            break;
        }
    }

    if (hasError) {
        strcpy(errorStr, "Error: Input contains non-alphanumeric characters.");
    } else {
        alphabetStr[alphaIndex] = '\0';
        digitStr[digitIndex] = '\0';
    }
}

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
    serverAddr.sin_port = htons(port); //bind the socket to the given port

    // Bind the socket to the port
    if (bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Bind error: ");
        return 0;
    }

    printf("Server is running at port: %d\n", port);

    char buffer[MAX_BUFF_SIZE];
    char alphabetStr[MAX_BUFF_SIZE];
    char digitStr[MAX_BUFF_SIZE];
    char errorStr[] = "Error: Input contains non-alphanumeric characters.";
    
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&clientAddr, &clientAddrLen);

        //check reveived data
        if (n < 0) {
            perror("Receive error");
            continue;
        }


        // int alphaIndex = 0;
        // int digitIndex = 0;
        // int hasError = 0;

        // for (int i = 0; i < n; i++) {
        //     if (isalpha(buffer[i])) {
        //         alphabetStr[alphaIndex++] = buffer[i];
        //     } else if (isdigit(buffer[i])) {
        //         digitStr[digitIndex++] = buffer[i];
        //     } else {
        //         hasError = 1;
        //         break;
        //     }
        // }

        if (strlen(errorStr) > 0) {
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
