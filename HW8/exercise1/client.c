#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s IP_Addr Port_Number\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *serverIP = argv[1];
    int port = atoi(argv[2]);
    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Invalid port number\n");
        exit(EXIT_FAILURE);
    }

    // Create socket
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set up server address struct
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(serverIP);
    serverAddr.sin_port = htons(port);

    // Connect to the server
    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Connection failed");
        close(clientSocket);
        exit(EXIT_FAILURE);
    }

    char inputBuffer[MAX_BUFFER_SIZE];
    char outputBuffer[MAX_BUFFER_SIZE];

    while (1) {
        // Get user input
        printf("Enter a string (blank to exit): ");
        fgets(inputBuffer, sizeof(inputBuffer), stdin);

        // Remove the newline character from the input
        inputBuffer[strcspn(inputBuffer, "\n")] = '\0';

        // Send the input string to the server
        send(clientSocket, inputBuffer, strlen(inputBuffer), 0);

        // Break the loop on a blank string
        if (strcmp(inputBuffer, "") == 0) {
            break;
        }

        // Receive the results from the server
        ssize_t bytesRead = recv(clientSocket, outputBuffer, sizeof(outputBuffer), 0);

        if (bytesRead <= 0) {
            break; // Connection closed or error
        }

        outputBuffer[bytesRead] = '\0';

        // Print the results
        printf("Alphabetic characters: %s\n", outputBuffer);

        bytesRead = recv(clientSocket, outputBuffer, sizeof(outputBuffer), 0);

        if (bytesRead <= 0) {
            break; // Connection closed or error
        }

        outputBuffer[bytesRead] = '\0';

        printf("Numeric characters: %s\n", outputBuffer);

        bytesRead = recv(clientSocket, outputBuffer, sizeof(outputBuffer), 0);

        if (bytesRead <= 0) {
            break; // Connection closed or error
        }

        outputBuffer[bytesRead] = '\0';

        printf("Undefined character count: %s\n", outputBuffer);
    }

    // Close the client socket
    close(clientSocket);

    return 0;
}
