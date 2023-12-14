#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

#define BUFFER_SIZE 1024

void resolveAndPrint(char *input) {
    struct hostent *host;
    struct in_addr addr;

    // Try to resolve as IP address
    if (inet_pton(AF_INET, input, &addr) > 0) {
        host = gethostbyaddr(&addr, sizeof(addr), AF_INET);
        if (host == NULL) {
            printf("Official name: (null)\n");
        } else {
            printf("Official name: %s\n", host->h_name);
        }

        char **alias = host->h_aliases;
        printf("Alias name:\n");
        while (*alias != NULL) {
            printf("%s\n", *alias);
            alias++;
        }
    }
    // Try to resolve as domain name
    else {
        host = gethostbyname(input);
        if (host == NULL) {
            printf("Not found information\n");
        } else {
            printf("Official IP: %s\n", inet_ntoa(*(struct in_addr*)host->h_addr_list[0]));

            char **alias = host->h_addr_list + 1;
            printf("Alias IP:\n");
            while (*alias != NULL) {
                printf("%s\n", inet_ntoa(*(struct in_addr*)*alias));
                alias++;
            }
        }
    }
}

void initializeClient(const char *serverIP, int port) {
    int clientSocket;
    struct sockaddr_in serverAddr;

    // Create socket
    if ((clientSocket = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set up server address struct
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(serverIP);
    serverAddr.sin_port = htons(port);

    char userInput[BUFFER_SIZE];

    // Get user input and send to server until a blank string is entered
    while (1) {
        printf("Enter a string (blank to exit): ");
        fgets(userInput, sizeof(userInput), stdin);

        // Remove newline character from the input
        size_t len = strlen(userInput);
        if (len > 0 && userInput[len - 1] == '\n') {
            userInput[len - 1] = '\0';
        }

        // Break the loop if the user enters a blank string
        if (strlen(userInput) == 0) {
            break;
        }

        // Send user input to the server
        if (sendto(clientSocket, userInput, strlen(userInput), 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
            perror("Error sending data to server");
            close(clientSocket);
            exit(EXIT_FAILURE);
        }

        // Receive results from the server
        char buffer[BUFFER_SIZE];
        ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);

        if (bytesRead <= 0) {
            perror("Error receiving results from server");
            close(clientSocket);
            exit(EXIT_FAILURE);
        }

        buffer[bytesRead] = '\0';  // Null-terminate the received data

        // Print the received results
        printf("%s\n", buffer);
    }

    // Close the client socket
    close(clientSocket);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s IPAddress Port_Number\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *serverIP = argv[1];
    int port = atoi(argv[2]);

    initializeClient(serverIP, port);

    return 0;
}
