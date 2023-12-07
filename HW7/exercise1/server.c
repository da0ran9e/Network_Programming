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
#define BUFFER_SIZE 5120

void sendFileToSocket(int sockfd, const char* folderPath) {
    int sent, offset;
    ServerMessage newMsg;
    newMsg.opcode = 2;

    struct dirent* entry;
    struct stat entryStat;
    time_t lastModifiedTime = 0;
    char* lastModifiedFile = NULL;

    DIR* directory = opendir(folderPath);
    if (directory == NULL) {
        perror("Error opening directory");
        return;
    }

    while ((entry = readdir(directory)) != NULL) {
        char filePath[PATH_MAX];
        snprintf(filePath, sizeof(filePath), "%s/%s", folderPath, entry->d_name);

        if (stat(filePath, &entryStat) == 0) {
            if (S_ISREG(entryStat.st_mode) && entryStat.st_mtime > lastModifiedTime) {
                lastModifiedTime = entryStat.st_mtime;
                if (lastModifiedFile != NULL) {
                    free(lastModifiedFile);
                }
                lastModifiedFile = strdup(entry->d_name);
            }
        }
    }

    closedir(directory);

    FILE* fileData = fopen(lastModifiedFile, "rb");

    while ((offset = fread(newMsg.payload, 1, BUFFER_SIZE, fileData)) > 0) {
        newMsg.length = offset;
        newMsg.payload[offset] = '\0';
    }

    fclose(fileData);

    newMsg.length = 0;
    memset(newMsg.payload, 0, BUFFER_SIZE);
    sent = sendMessage(sockfd, &newMsg); 
}

void receiveFileFromSocket(int sockfd, int editMode, int key) {
    int bytesReceived;
    ServerMessage newMsg;
    char fileBuffer[BUFFER_SIZE] = {0}; // Initialize buffer

    while (1) {
        bytesReceived = recvMessage(sockfd, &newMsg);
        if (bytesReceived < 0) {
            break;
        }

        if (newMsg.length == 0) { // end of file
            break;
        }

        edit(newMsg.payload, editMode, key);
        strcat(fileBuffer, newMsg.payload);
    }

    save(fileBuffer);
}

void edit(char* content, int editMode, int key) {
    switch (editMode) {
        case 0:
            encrypt(content, key);
            break;
        case 1:
            decrypt(content, key);
            break;
        default:
            printf("ERROR: Invalid edit mode!\n");
            return;
    }
}

void handleMessage(int sockfd, ServerMessage* msg, const char* filename) {
    int key = atoi(msg->payload);
    int editMode = msg->opcode;

    receiveFileFromSocket(sockfd, editMode, key);

    sendFileToSocket(sockfd, filename);

    deleteFile(filename);
}

void run(int listeningSocket, const char* filename) {
    int maxi, maxfd;
    int client[FD_SETSIZE];
    fd_set readfds, allset;

    maxfd = listeningSocket;
    maxi = -1;

    for (int i = 0; i < FD_SETSIZE; i++)
        client[i] = -1;

    FD_ZERO(&allset);
    FD_SET(listeningSocket, &allset);

    socklen_t clilen;
    struct sockaddr_in cliaddr;

    int connfd, sockfd;
    int nready;

    while (1) {
        readfds = allset; /* structure assignment */
        nready = select(maxfd + 1, &readfds, NULL, NULL, NULL);
        
        if (nready < 0) {
            perror("\nError: ");
            return;
        }

        if (FD_ISSET(listeningSocket, &readfds)) { /* new client connection */
            clilen = sizeof(cliaddr);
            if ((connfd = accept(listeningSocket, (struct sockaddr*)&cliaddr, &clilen)) < 0) {
                perror("\nError: ");
            } else {
                printf("You got a connection from %s\n", inet_ntoa(cliaddr.sin_addr));
                int count;
                for (count = 0; count < FD_SETSIZE; count++)
                    if (client[count] < 0) {
                        client[count] = connfd;
                        break;
                    }
                if (count == FD_SETSIZE) {
                    printf("\nToo many clients");
                    close(connfd);
                }

                FD_SET(connfd, &allset);
                if (connfd > maxfd) {
                    maxfd = connfd;
                }

                if (count > maxi) {
                    maxi = count;
                }

                if (--nready <= 0)
                    continue; /* no more readable descriptors */
            }
        }

        ServerMessage receivedMessage;
        for (int i = 0; i <= maxi; i++) { /* check all clients for data */
            if ((sockfd = client[i]) < 0) {
                continue;
            }

            if (FD_ISSET(sockfd, &readfds)) {
                ssize_t bytesReceived = recvMessage(sockfd, &receivedMessage);
                if (bytesReceived <= 0) {
                    FD_CLR(sockfd, &allset);
                    close(sockfd);
                    client[i] = -1;
                } else {
                    handleMessage(sockfd, &receivedMessage, filename);
                }

                if (--nready <= 0)
                    break; /* no more readable descriptors */
            }
        }
    }
}

int initializeServer(int port) {
    int listeningSocket;
    struct sockaddr_in server;

    if ((listeningSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) { /* calls socket() */
        printf("socket() error\n");
        exit(1);
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = htonl(INADDR_ANY); /* INADDR_ANY puts your IP address automatically */

    if (bind(listeningSocket, (struct sockaddr*)&server, sizeof(server)) == -1) {
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

    const char* filename = "output.txt"; // Change this to your desired filename
    int listeningSocket;

    listeningSocket = initializeServer(atoi(argv[1]));

    run(listeningSocket, filename);
    close(listeningSocket);

    return 0;
}
