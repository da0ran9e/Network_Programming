#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>

#define MAX_BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s Port_Number\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t addrLen = sizeof(clientAddr);

    // Create socket
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set up server address struct
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(atoi(argv[1]));

    // Bind the socket to the specified port
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Bind failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(serverSocket, 5) == -1) {
        perror("Listen failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", atoi(argv[1]));

    while (1) {
        // Accept a connection
        if ((clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addrLen)) == -1) {
            perror("Accept failed");
            continue;
        }

        printf("Connection accepted from %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

        // Receive the string from the client
        char buffer[MAX_BUFFER_SIZE];
        ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);

        if (bytesRead <= 0) {
            perror("Error receiving data");
            close(clientSocket);
            continue;
        }

        // Process the received string
        buffer[bytesRead] = '\0';

        char resultAlpha[MAX_BUFFER_SIZE] = "";
        char resultDigit[MAX_BUFFER_SIZE] = "";
        int undefinedCount = 0;

        for (size_t i = 0; i < bytesRead; i++) {
            if (isalpha(buffer[i])) {
                strncat(resultAlpha, &buffer[i], 1);
            } else if (isdigit(buffer[i])) {
                strncat(resultDigit, &buffer[i], 1);
            } else {
                undefinedCount++;
            }
        }

        // Send the results back to the client
        struct iovec iov[3];
        iov[0].iov_base = resultAlpha;
        iov[0].iov_len = strlen(resultAlpha);
        iov[1].iov_base = resultDigit;
        iov[1].iov_len = strlen(resultDigit);
        iov[2].iov_base = &undefinedCount;
        iov[2].iov_len = sizeof(int);

        ssize_t bytesSent = writev(clientSocket, iov, 3);

        if (bytesSent == -1) {
            perror("Error sending results");
        }

        // Close the client socket
        close(clientSocket);
    }

    // Close the server socket
    close(serverSocket);

    return 0;
}
