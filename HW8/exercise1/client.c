#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFFER_SIZE 1024

void sendString(int socket, const char *str) {
    send(socket, str, strlen(str), 0);
}

void receiveResults(int socket) {
    char buffer[BUFFER_SIZE];
    ssize_t bytesRead;

    // Receive results using readv for simultaneous reception
    struct iovec iov[3];
    iov[0].iov_base = buffer;
    iov[0].iov_len = sizeof(buffer);

    bytesRead = readv(socket, iov, 1);

    if (bytesRead <= 0) {
        perror("Error receiving results");
        close(socket);
        exit(EXIT_FAILURE);
    }

    buffer[bytesRead] = '\0';
    printf("%s", buffer);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s IP_Addr Port_Number\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *serverIP = argv[1];
    int port = atoi(argv[2]);
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
    serverAddr.sin_addr.s_addr = inet_addr(serverIP);
    serverAddr.sin_port = htons(port);

    // Connect to the server
    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Connection failed");
        close(clientSocket);
        exit(EXIT_FAILURE);
    }

    char inputString[BUFFER_SIZE];

    do {
        // Get user input
        printf("Enter a string (or type a blank string to exit): ");
        fgets(inputString, sizeof(inputString), stdin);

        // Send the input string to the server
        sendString(clientSocket, inputString);

        if (strlen(inputString) > 1) {
            // Receive and print the results from the server
            receiveResults(clientSocket);
        }
    } while (strlen(inputString) > 1);

    // Close the client socket
    close(clientSocket);

    return 0;
}
