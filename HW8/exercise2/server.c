#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>

#define BUFFER_SIZE 1024

void handleSignal(int signo) {
    // Signal handler to handle SIGIO
    printf("Received SIGIO\n");
}

void initializeServer(int port) {
    int serverSocket;
    struct sockaddr_in serverAddr;

    // Create socket
    if ((serverSocket = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set up server address struct
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    // Bind the socket to the specified port
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Bind failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    // Set signal-driven I/O mode
    if (fcntl(serverSocket, F_SETOWN, getpid()) == -1) {
        perror("fcntl F_SETOWN failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    int flags = fcntl(serverSocket, F_GETFL);
    if (fcntl(serverSocket, F_SETFL, flags | O_ASYNC | O_NONBLOCK) == -1) {
        perror("fcntl F_SETFL failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    // Set signal handler for SIGIO
    if (signal(SIGIO, handleSignal) == SIG_ERR) {
        perror("signal(SIGIO) failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    // Enable asynchronous I/O
    if (fcntl(serverSocket, F_SETSIG, SIGIO) == -1) {
        perror("fcntl F_SETSIG failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    if (fcntl(serverSocket, F_SETFL, O_ASYNC | O_NONBLOCK) == -1) {
        perror("fcntl F_SETFL (O_ASYNC) failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", port);

    while (1) {
        sleep(1);  // Server will process signals in the signal handler
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s Port_Number\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    initializeServer(port);

    return 0;
}
