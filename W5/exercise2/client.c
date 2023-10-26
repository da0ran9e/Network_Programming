#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_BUFFER_SIZE 1024
#define MAX_FILENAME_SIZE 128

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <IP_Addr> <Port_Number>\n", argv[0]);
        exit(1);
    }

    int clientSocket;
    char buffer[MAX_BUFFER_SIZE];

    struct sockaddr_in serverAddr;

    // Create client socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        perror("socket");
        exit(1);
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(atoi(argv[2]));

    if (inet_pton(AF_INET, argv[1], &serverAddr.sin_addr) <= 0) {
        perror("inet_pton");
        close(clientSocket);
        exit(1);
    }

    // Connect to the server
    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("connect");
        close(clientSocket);
        exit(1);
    }

    printf("Connected to server %s:%s\n", argv[1], argv[2]);

    while (1) {
        printf("Enter file path (blank to exit): ");
        fgets(buffer, MAX_FILENAME_SIZE, stdin);

        // Remove newline character from input
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }

        if (strlen(buffer) == 0) {
            break; // Exit if the input is blank
        }

        char filepath[MAX_BUFFER_SIZE] = "cliennt_folder/";
        strcat(filepath, buffer);

        // Check if file exists
        if (access(filepath, F_OK) == -1) {
            fprintf(stderr, "Error: File '%s' not found.\n", buffer);
            continue;
        }

        // Send file name to server
        if (send(clientSocket, buffer, strlen(buffer), 0) == -1) {
            perror("send");
            break;
        }

        // Open and send file data
        FILE *file = fopen(filepath, "rb");
        if (file == NULL) {
            perror("fopen");
            break;
        }

        while (1) {
            size_t bytesRead = fread(buffer, 1, MAX_BUFFER_SIZE, file);
            if (bytesRead == 0) {
                if (feof(file)) {
                    break;
                } else {
                    perror("fread");
                    break;
                }
            }

            if (send(clientSocket, buffer, bytesRead, 0) == -1) {
                perror("send");
                break;
            }
        }

        fclose(file);
        printf("File '%s' sent to server.\n", buffer);
    }

    close(clientSocket);

    return 0;
}
