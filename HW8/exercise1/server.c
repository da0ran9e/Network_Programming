#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

void processString(const char* input, char* alphabet, char* digits, int* undefinedCount) {
    int alphIndex = 0;
    int digitIndex = 0;
    *undefinedCount = 0;

    for (int i = 0; input[i] != '\0'; i++) {
        if (isalpha(input[i])) {
            alphabet[alphIndex++] = input[i];
        } else if (isdigit(input[i])) {
            digits[digitIndex++] = input[i];
        } else {
            (*undefinedCount)++;
        }
    }

    alphabet[alphIndex] = '\0';
    digits[digitIndex] = '\0';
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s Port_Number\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t addrLen = sizeof(clientAddr);
    char buffer[BUFFER_SIZE];
    char alphabet[BUFFER_SIZE];
    char digits[BUFFER_SIZE];

    int port = atoi(argv[1]);

    // Create socket
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set up server address struct
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

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

    printf("Server listening on port %d...\n", port);

    // Accept incoming connections and process client requests
    while (1) {
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addrLen);

        if (clientSocket == -1) {
            perror("Accept failed");
            continue;
        }

        printf("Connection accepted from %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

        // Set client socket to non-blocking mode
        fcntl(clientSocket, F_SETFL, O_NONBLOCK);

        int bytesRead = 0;
        int totalBytesRead = 0;

        while ((bytesRead = read(clientSocket, buffer + totalBytesRead, sizeof(buffer) - totalBytesRead)) > 0) {
            totalBytesRead += bytesRead;
        }

        if (bytesRead == 0) {
            // Client disconnected
            printf("Client disconnected\n");
            close(clientSocket);
            continue;
        } else if (bytesRead == -1 && errno != EWOULDBLOCK) {
            perror("Error reading from client");
            close(clientSocket);
            continue;
        }

        // Null-terminate the received data
        buffer[totalBytesRead] = '\0';

        // Process the received string
        int undefinedCount;
        processString(buffer, alphabet, digits, &undefinedCount);

        // Send results back to the client
        char resultBuffer[BUFFER_SIZE];
        snprintf(resultBuffer, sizeof(resultBuffer), "Alphabets: %s\nDigits: %s\nUndefined characters count: %d\n",
                 alphabet, digits, undefinedCount);

        // Write the results back to the client
        write(clientSocket, resultBuffer, strlen(resultBuffer));

        // Close the client socket
        close(clientSocket);
    }

    // Close the server socket
    close(serverSocket);

    return 0;
}
