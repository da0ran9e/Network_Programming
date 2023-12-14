#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <netdb.h>

#define BUFFER_SIZE 1024

void handleSignal(int signal) {
    printf("Received signal %d. Exiting.\n", signal);
    exit(EXIT_SUCCESS);
}

void processClientData(int clientSocket) {
    char buffer[BUFFER_SIZE];
    char resultBuffer[BUFFER_SIZE];
    struct hostent *host;
    struct in_addr addr;

    // Receive data from the client
    ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);

    if (bytesRead <= 0) {
        perror("Error receiving data from client");
        return;
    }

    buffer[bytesRead] = '\0';  // Null-terminate the received data

    // Convert IP address or domain name to the opposite
    if (inet_pton(AF_INET, buffer, &addr)) {
        // Input is an IP address, get domain name
        host = gethostbyaddr(&addr, sizeof(addr), AF_INET);

        if (host != NULL) {
            snprintf(resultBuffer, sizeof(resultBuffer), "Official name: %s\nAlias name: %s\n", host->h_name, host->h_aliases[0]);
        } else {
            snprintf(resultBuffer, sizeof(resultBuffer), "Not found information\n");
        }
    } else {
        // Input is a domain name, get IP addresses
        host = gethostbyname(buffer);

        if (host != NULL) {
            int i = 0;
            char aliasBuffer[BUFFER_SIZE];
            strcpy(resultBuffer, "Official IP: ");
            while (host->h_addr_list[i] != NULL) {
                if (i > 0) {
                    strcat(resultBuffer, "\nAlias IP:\n");
                    strcpy(aliasBuffer, "");
                }
                strcat(resultBuffer, inet_ntoa(*(struct in_addr*)host->h_addr_list[i]));
                strcat(aliasBuffer, inet_ntoa(*(struct in_addr*)host->h_addr_list[i]));
                i++;
            }
            if (i > 1) {
                strcat(resultBuffer, aliasBuffer);
            }
            strcat(resultBuffer, "\n");
        } else {
            snprintf(resultBuffer, sizeof(resultBuffer), "Not found information\n");
        }
    }

    // Send the result back to the client
    send(clientSocket, resultBuffer, strlen(resultBuffer), 0);
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

    // Register signal handler for termination
    signal(SIGINT, handleSignal);

    printf("Server listening on port %d...\n", atoi(argv[1]));

    // Receive and process client data
    while (1) {
        char buffer[BUFFER_SIZE];
        int bytesRead = recvfrom(serverSocket, buffer, sizeof(buffer), 0, (struct sockaddr*)&clientAddr, &addrLen);

        if (bytesRead <= 0) {
            perror("Error receiving data from client");
            continue;
        }

        buffer[bytesRead] = '\0';  // Null-terminate the received data

        printf("Received data from %s:%d: %s\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), buffer);

        // Process client data
        processClientData(serverSocket, (struct sockaddr*)&clientAddr, addrLen, buffer);
    }

    // Close the server socket
    close(serverSocket);

    return 0;
}
