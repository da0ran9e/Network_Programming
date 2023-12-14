#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s IP_Addr Port_Number\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int clientSocket;
    struct sockaddr_in serverAddr;
    char inputBuffer[BUFFER_SIZE];

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
        printf("Enter a string (or press Enter to quit): ");
        if (fgets(inputBuffer, sizeof(inputBuffer), stdin) == NULL || inputBuffer[0] == '\n') {
            break;
        }

        // Send the user's input to the server
        write(clientSocket, inputBuffer, strlen(inputBuffer));

        // Read and print the results from the server
        char resultBuffer[BUFFER_SIZE];
        read(clientSocket, resultBuffer, sizeof(resultBuffer));
        printf("Server response:\n%s", resultBuffer);
    }

    // Close the client socket
    close(clientSocket);

    return 0;
}
