#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <sys/select.h> 
#include <dirent.h>

#include "server/include/protocol.h"
#include "server/include/file_handler.h"
#include "server/include/caesar.h"

#define BACKLOG 20

#define MAX_BUFFER_SIZE 5120

void sendLatestFileToSocket(int clientSocket) {
    int sent, bytesRead;
    ServerMessage message;
    message.opcode = 2;

    char* latestFileName = getLatestFileName();
    FILE* file = fopen(latestFileName, "rb");

    while ((bytesRead = fread(message.payload, 1, BUFFER_SIZE, file)) > 0) {
        message.length = bytesRead;
        message.payload[bytesRead] = '\0';
        sent = sendMessage(clientSocket, &message);
    }

    fclose(file);
    free(latestFileName);

    message.length = 0;
    memset(message.payload, 0, BUFFER_SIZE);

    sent = sendMessage(clientSocket, &message);
}

void receiveFileFromSocket(int clientSocket, int editMode, int encryptionKey) {
    int bytesRead;
    ServerMessage message;
    char fileBuffer[MAX_BUFFER_SIZE] = {0};

    while (1) {
        bytesRead = recvMessage(clientSocket, &message);

        if (bytesRead < 0 || message.length == 0) {
            break;
        }

        applyEditOperation(message.payload, editMode, encryptionKey);
        strcat(fileBuffer, message.payload);
    }

    save(fileBuffer);
}

void applyEditOperation(char* content, int editMode, int key) {
    switch (editMode) {
        case 0:
            encrypt(content, key);
            break;
        case 1:
            decrypt(content, key);
            break;
        default:
            printf("ERROR: Invalid opcode!\n");
            return;
    }
}

void handleMessage(int clientSocket, ServerMessage* receivedMessage) {
    int encryptionKey = atoi(receivedMessage->payload);
    int editMode = receivedMessage->opcode;

    receiveFileFromSocket(clientSocket, editMode, encryptionKey);
    sendLatestFileToSocket(clientSocket);
    deleteFile(getLatestFileName());
}

void runServer(int listeningSocket) {
    int maxFileDescriptor, maxIndex, newClientSocket;
    int clientSockets[FD_SETSIZE];
    fd_set allSet, readSet;

    maxFileDescriptor = listeningSocket;
    maxIndex = -1;

    for (int i = 0; i < FD_SETSIZE; i++)
        clientSockets[i] = -1;

    FD_ZERO(&allSet);
    FD_SET(listeningSocket, &allSet);

    socklen_t clientAddressLength;
    struct sockaddr_in clientAddress;

    int nReady;
    while (1) {
        readSet = allSet;

        nReady = select(maxFileDescriptor + 1, &readSet, NULL, NULL, NULL);

        if (nReady < 0) {
            perror("\nError: ");
            return;
        }

        if (FD_ISSET(listeningSocket, &readSet)) {
            clientAddressLength = sizeof(clientAddress);

            if ((newClientSocket = accept(listeningSocket, (struct sockaddr*)&clientAddress, &clientAddressLength)) < 0) {
                perror("\nError: ");
            } else {
                handleNewConnection(newClientSocket, &allSet, clientSockets, &maxFileDescriptor, &maxIndex);

                if (--nReady <= 0) {
                    continue;  // No more readable descriptors
                }
            }
        }

        handleClientMessages(clientSockets, &readSet, &nReady);
    }
}

void handleNewConnection(int newClientSocket, fd_set* allSet, int clientSockets[], int* maxFileDescriptor, int* maxIndex) {
    printf("New connection from %s\n", getClientAddress(newClientSocket));  // Function not provided; replace with your implementation

    int i;
    for (i = 0; i < FD_SETSIZE; i++) {
        if (clientSockets[i] < 0) {
            clientSockets[i] = newClientSocket;
            break;
        }
    }

    if (i == FD_SETSIZE) {
        printf("\nToo many clients");
        close(newClientSocket);
    }

    FD_SET(newClientSocket, allSet);

    if (newClientSocket > *maxFileDescriptor) {
        *maxFileDescriptor = newClientSocket;
    }

    if (i > *maxIndex) {
        *maxIndex = i;
    }
}

void handleClientMessages(int clientSockets[], fd_set* readSet, int* nReady) {
    ServerMessage receivedMessage;

    for (int i = 0; i <= *maxIndex; i++) {
        int clientSocket = clientSockets[i];

        if (clientSocket < 0) {
            continue;
        }

        if (FD_ISSET(clientSocket, readSet)) {
            ssize_t bytesRead = recvMessage(clientSocket, &receivedMessage);

            if (bytesRead <= 0) {
                closeClientSocket(clientSocket, readSet, clientSockets);
            } else {
                handleMessage(clientSocket, &receivedMessage);

                if (--(*nReady) <= 0) {
                    break;  // No more readable descriptors
                }
            }
        }
    }
}

void closeClientSocket(int clientSocket, fd_set* allSet, int clientSockets[]) {
    FD_CLR(clientSocket, allSet);
    close(clientSocket);
    clientSockets[i] = -1;
}

int initializeServer(int port) {
    int listeningSocket;
    struct sockaddr_in serverAddress;

    if ((listeningSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("socket() error\n");
        exit(1);
    }

    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listeningSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        perror("\nError bind: ");
        close(listeningSocket);
        exit(1);
    }

    if (listen(listeningSocket, BACKLOG) == -1) {
        perror("\nError listen: ");
        close(listeningSocket);
        exit(1);
    }

    printf("Server created.\n\n");
    return listeningSocket;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s PortNumber\n", argv[0]);
        exit(1);
    }

    int listeningSocket;

    listeningSocket = initializeServer(atoi(argv[1]));

    runServer(listeningSocket);

    close(listeningSocket);

    return 0;
}
