#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>

#define BUFFER_SIZE 1024

void handleSignal(int signal) {
    // Handle signals here if needed
}

void resolveAndSendResult(int clientSocket, const char *input) {
    struct addrinfo hints, *result, *p;
    char buffer[BUFFER_SIZE];

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    // Resolve the input (domain name or IP address)
    if (getaddrinfo(input, NULL, &hints, &result) != 0) {
        snprintf(buffer, sizeof(buffer), "Not found information\n");
        sendto(clientSocket, buffer, strlen(buffer), 0, NULL, 0);
        return;
    }

    // Iterate through the list and send the result back to the client
    for (p = result; p != NULL; p = p->ai_next) {
        char host[INET6_ADDRSTRLEN];

        if (getnameinfo(p->ai_addr, p->ai_addrlen, host, sizeof(host), NULL, 0, NI_NUMERICHOST) != 0) {
            perror("getnameinfo failed");
            continue;
        }

        snprintf(buffer, sizeof(buffer), "%s\n", host);
        sendto(clientSocket, buffer, strlen(buffer), 0, NULL, 0);
    }

    freeaddrinfo(result);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s Port_Number\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int serverSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t addrLen = sizeof(clientAddr);

    // Create socket
    if ((serverSocket = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
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

    printf("Server listening on port %d...\n", atoi(argv[1]));

    // Register signal handler
    signal(SIGINT, handleSignal);

    while (1) {
        char buffer[BUFFER_SIZE];
        ssize_t bytesRead;

        // Receive data from the client
        bytesRead = recvfrom(serverSocket, buffer, sizeof(buffer), 0, (struct sockaddr*)&clientAddr, &addrLen);

        if (bytesRead <= 0) {
            perror("Error receiving data from client");
            continue;
        }

        buffer[bytesRead] = '\0';  // Null-terminate the received data

        // Process client request and send the result
        resolveAndSendResult(serverSocket, buffer);
    }

    // Close the server socket
    close(serverSocket);

    return 0;
}
