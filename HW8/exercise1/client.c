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

    int clientSocket;
    struct sockaddr_in serverAddr;

    // Create socket
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set up server address struct
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(argv[1]); // Server IP address
    serverAddr.sin_port = htons(atoi(argv[2]));

    // Connect to the server
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Connection failed");
        close(clientSocket);
        exit(EXIT_FAILURE);
    }

    while (1) {
        // Get user input
        printf("Enter a string (or press Enter to exit): ");
        char inputBuffer[MAX_BUFFER_SIZE];
        fgets(inputBuffer, sizeof(inputBuffer), stdin);

        // Remove newline character from input
        size_t inputLength = strlen(inputBuffer);
        if (inputBuffer[inputLength - 1] == '\n') {
            inputBuffer[inputLength - 1] = '\0';
        }

        // Break the loop if the user enters a blank string
        if (inputLength == 1 && inputBuffer[0] == '\0') {
            break;
        }

        // Send the input string to the server
        ssize_t bytesSent = send(clientSocket, inputBuffer, strlen(inputBuffer), 0);

        if (bytesSent == -1) {
            perror("Error sending data");
            close(clientSocket);
            exit(EXIT_FAILURE);
        }

        // Receive results from the server
        char resultAlpha[MAX_BUFFER_SIZE];
        char resultDigit[MAX_BUFFER_SIZE];
        int undefinedCount;

        struct iovec iov[3];
        iov[0].iov_base = resultAlpha;
        iov[0].iov_len = sizeof(resultAlpha);
        iov[1].iov_base = resultDigit;
        iov[1].iov_len = sizeof(resultDigit);
        iov[2].iov_base = &undefinedCount;
        iov[2].iov_len = sizeof(int);

        ssize_t bytesRead = readv(clientSocket, iov, 3);

        if (bytesRead == -1) {
            perror("Error receiving results");
            close(clientSocket);
            exit(EXIT_FAILURE);
        }

        // Null-terminate the received data
        resultAlpha[iov[0].iov_len] = '\0';
        resultDigit[iov[1].iov_len] = '\0';

        // Print the results
        printf("Result (Alphabets): %s\n", resultAlpha);
        printf("Result (Digits): %s\n", resultDigit);

        if (undefinedCount > 0) {
            printf("Total undefined characters: %d\n", undefinedCount);
        }
    }

    // Close the client socket
    close(clientSocket);

    return 0;
}
