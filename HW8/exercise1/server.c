#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <ctype.h>

#define MAX_CLIENTS 30
#define BUFFER_SIZE 1024

void processClientData(int clientSocket) {
    char buffer[BUFFER_SIZE];
    ssize_t bytesRead;

    // Receive data from the client
    bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);

    if (bytesRead <= 0) {
        // Handle disconnection or error
        close(clientSocket);
        printf("Client disconnected\n");
        return;
    }

    buffer[bytesRead] = '\0';

    // Process the received data
    char alphabetString[BUFFER_SIZE] = "";
    char digitString[BUFFER_SIZE] = "";
    int undefinedCount = 0;

    for (size_t i = 0; i < bytesRead; ++i) {
        if (isalpha(buffer[i])) {
            strncat(alphabetString, &buffer[i], 1);
        } else if (isdigit(buffer[i])) {
            strncat(digitString, &buffer[i], 1);
        } else {
            undefinedCount++;
        }
    }

    // Send the results back to the client
    send(clientSocket, digitString, strlen(digitString), 0);
    send(clientSocket, alphabetString, strlen(alphabetString), 0);

    // If there are undefined characters, send the count
    if (undefinedCount > 0) {
        char undefinedCountStr[BUFFER_SIZE];
        snprintf(undefinedCountStr, sizeof(undefinedCountStr), "There is %d undefined character\n", undefinedCount);
        send(clientSocket, undefinedCountStr, strlen(undefinedCountStr), 0);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s Port_Number\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    int serverSocket, clientSocket, maxfd;
    fd_set readfds;
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
    serverAddr.sin_port = htons(port);

    // Bind the socket to the specified port
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
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

    FD_ZERO(&readfds);
    FD_SET(serverSocket, &readfds);
    maxfd = serverSocket;

    while (1) {
        fd_set tempfds = readfds;
        int activity = select(maxfd + 1, &tempfds, NULL, NULL, NULL);

        if (activity < 0) {
            perror("Select error");
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(serverSocket, &tempfds)) {
            // New connection
            if ((clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &addrLen)) == -1) {
                perror("Accept failed");
                continue;
            }

            printf("New connection from %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

            FD_SET(clientSocket, &readfds);

            if (clientSocket > maxfd) {
                maxfd = clientSocket;
            }
        }

        // Check data from clients
        for (int i = serverSocket + 1; i <= maxfd; i++) {
            if (FD_ISSET(i, &tempfds)) {
                processClientData(i);
                FD_CLR(i, &readfds);
                close(i);
            }
        }
    }

    // Close the server socket
    close(serverSocket);

    return 0;
}
