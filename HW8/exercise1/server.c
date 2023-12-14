#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>

#define BUFFER_SIZE 1024

void processClientData(int clientSocket) {
    char buffer[BUFFER_SIZE];
    char alphabetString[BUFFER_SIZE];
    char digitString[BUFFER_SIZE];

    // Read data from the client
    ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);

    if (bytesRead <= 0) {
        perror("Error reading from client");
        return;
    }

    buffer[bytesRead] = '\0';  // Null-terminate the received data

    // Process the received string
    int alphabetIndex = 0;
    int digitIndex = 0;
    int undefinedCount = 0;

    for (int i = 0; i < bytesRead; i++) {
        if (isalpha(buffer[i])) {
            alphabetString[alphabetIndex++] = buffer[i];
        } else if (isdigit(buffer[i])) {
            digitString[digitIndex++] = buffer[i];
        } else {
            undefinedCount++;
        }
    }

    alphabetString[alphabetIndex] = '\0';  // Null-terminate the alphabet string
    digitString[digitIndex] = '\0';        // Null-terminate the digit string

    // Send the processed strings and the count of undefined characters back to the client
    send(clientSocket, alphabetString, strlen(alphabetString), 0);
    send(clientSocket, digitString, strlen(digitString), 0);

    if (undefinedCount > 0) {
        char undefinedCountStr[20];
        snprintf(undefinedCountStr, sizeof(undefinedCountStr), "There is %d undefined character\n", undefinedCount);
        send(clientSocket, undefinedCountStr, strlen(undefinedCountStr), 0);
    }
}

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

    // Accept incoming connections and process client data
    while (1) {
        // Use select to check if the socket is ready for reading
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(serverSocket, &readfds);

        if (select(serverSocket + 1, &readfds, NULL, NULL, NULL) == -1) {
            perror("Select failed");
            close(serverSocket);
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(serverSocket, &readfds)) {
            if ((clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addrLen)) == -1) {
                perror("Accept failed");
                continue;
            }

            printf("Connection accepted from %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

            // Process client data
            processClientData(clientSocket);

            // Close the client socket
            close(clientSocket);
        }
    }

    // Close the server socket
    close(serverSocket);

    return 0;
}
