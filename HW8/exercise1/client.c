#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

void sendToServer(int serverSocket, const char *data) {
    ssize_t bytesSent = send(serverSocket, data, strlen(data), 0);

    if (bytesSent == -1) {
        perror("Error sending data to server");
        exit(EXIT_FAILURE);
    }
}

void receiveResults(int serverSocket) {
    char buffer[BUFFER_SIZE];

    ssize_t bytesRead = recv(serverSocket, buffer, sizeof(buffer), 0);

    if (bytesRead <= 0) {
        perror("Error receiving results from server");
        exit(EXIT_FAILURE);
    }

    buffer[bytesRead] = '\0';

    printf("%s\n", buffer);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s IP_Addr Port_Number\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int clientSocket;
    struct sockaddr_in serverAddr;

    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(argv[1]);
    serverAddr.sin_port = htons(atoi(argv[2]));

    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Connection failed");
        close(clientSocket);
        exit(EXIT_FAILURE);
    }

    char userInput[BUFFER_SIZE];

    while (1) {
        printf("Enter a string (blank to exit): ");
        fgets(userInput, sizeof(userInput), stdin);

        size_t len = strlen(userInput);
        if (len > 0 && userInput[len - 1] == '\n') {
            userInput[len - 1] = '\0';
        }

        if (strlen(userInput) == 0) {
            break;
        }

        sendToServer(clientSocket, userInput);

        receiveResults(clientSocket);
    }

    close(clientSocket);

    return 0;
}
