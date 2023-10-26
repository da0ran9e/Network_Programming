#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_BUFFER_SIZE 1024
#define MAX_FILENAME_SIZE 128

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <Port_Number>\n", argv[0]);
        exit(1);
    }

    int serverSocket, clientSocket;
    char buffer[MAX_BUFFER_SIZE];

    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientLen = sizeof(clientAddr);

    // Create server socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("socket");
        exit(1);
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(atoi(argv[1]));

    // Bind server socket
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("bind");
        close(serverSocket);
        exit(1);
    }

    // Listen for incoming connections
    if (listen(serverSocket, 5) == -1) {
        perror("listen");
        close(serverSocket);
        exit(1);
    }

    printf("Server is running. Waiting for connections...\n");

    while (1) {
        // Accept client connection
        clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientLen);
        if (clientSocket == -1) {
            perror("accept");
            close(serverSocket);
            exit(1);
        }

        memset(buffer, 0, MAX_BUFFER_SIZE);

        // Receive file name from client
        ssize_t bytesRead = recv(clientSocket, buffer, MAX_FILENAME_SIZE, 0);
        if (bytesRead <= 0) {
            perror("recv");
            close(clientSocket);
            continue;
        }

        // Check if file already exists
        if (access(buffer, F_OK) != -1) {
            fprintf(stderr, "Error: File '%s' already exists on the server.\n", buffer);
            close(clientSocket);
            continue;
        }

        // Create and open the file for writing
        FILE *file = fopen(buffer, "wb");
        if (file == NULL) {
            perror("fopen");
            close(clientSocket);
            continue;
        }

        // Receive and write file data
        while (1) {
            bytesRead = recv(clientSocket, buffer, MAX_BUFFER_SIZE, 0);
            if (bytesRead <= 0) {
                perror("recv");
                fclose(file);
                close(clientSocket);
                break;
            }
            fwrite(buffer, 1, bytesRead, file);
        }

        fclose(file);
        printf("File '%s' received and saved.\n", buffer);
        close(clientSocket);
    }

    close(serverSocket);

    return 0;
}
