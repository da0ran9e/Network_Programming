#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <arpa/inet.h>

#define MAX_BUFFER_SIZE 1024

void processString(const char *input, char *alphaString, char *digitString, int *undefinedCount) {
    int alphaIndex = 0, digitIndex = 0;
    *undefinedCount = 0;

    for (size_t i = 0; i < strlen(input); i++) {
        char ch = input[i];
        if (isalpha(ch)) {
            alphaString[alphaIndex++] = ch;
        } else if (isdigit(ch)) {
            digitString[digitIndex++] = ch;
        } else {
            (*undefinedCount)++;
        }
    }

    alphaString[alphaIndex] = '\0';
    digitString[digitIndex] = '\0';
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s Port_Number\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Invalid port number\n");
        exit(EXIT_FAILURE);
    }

    // Create socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set up server address struct
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    // Bind the socket to the specified port
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Bind failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(serverSocket, 1) == -1) {
        perror("Listen failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    printf("Server started. Listening on port %d...\n", port);

    // Accept incoming connections
    int clientSocket = accept(serverSocket, NULL, NULL);
    if (clientSocket == -1) {
        perror("Accept failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    char inputBuffer[MAX_BUFFER_SIZE];
    char alphaBuffer[MAX_BUFFER_SIZE];
    char digitBuffer[MAX_BUFFER_SIZE];
    int undefinedCount;

    while (1) {
        // Receive a string from the client
        ssize_t bytesRead = recv(clientSocket, inputBuffer, sizeof(inputBuffer), 0);

        if (bytesRead <= 0) {
            break; // Connection closed or error
        }

        inputBuffer[bytesRead] = '\0';

        if (strcmp(inputBuffer, "") == 0) {
            break; // Break the loop on a blank string
        }

        // Process the string
        processString(inputBuffer, alphaBuffer, digitBuffer, &undefinedCount);

        // Send the results back to the client
        send(clientSocket, alphaBuffer, strlen(alphaBuffer), 0);
        send(clientSocket, digitBuffer, strlen(digitBuffer), 0);

        // Send the count of undefined characters
        char undefinedCountStr[10];
        snprintf(undefinedCountStr, sizeof(undefinedCountStr), "%d", undefinedCount);
        send(clientSocket, undefinedCountStr, strlen(undefinedCountStr), 0);
    }

    // Close sockets
    close(clientSocket);
    close(serverSocket);

    return 0;
}
