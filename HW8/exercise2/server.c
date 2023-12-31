#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>

#define MAX_BUFF_SIZE 1024

int g_serverSocket; 

void sig_io(int signo) {
    int n;
    struct sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);
    char buffer[MAX_BUFF_SIZE];

    n = recvfrom(g_serverSocket, buffer, sizeof(buffer), 0, (struct sockaddr*)&clientAddr, &addrLen);

    if (n < 0) {
        perror("Error receiving data");
        return;
    }

    buffer[n] = '\0';

    // Process the received domain name or IP address
    //use getaddrinfo to obtain information about the input
    struct addrinfo hints, *res;
    int status;
    char result[MAX_BUFF_SIZE];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    status = getaddrinfo(buffer, NULL, &hints, &res);

    if (status != 0) {
        snprintf(result, sizeof(result), "Not found information\n");
    } else {
        struct addrinfo* p;
        for (p = res; p != NULL; p = p->ai_next) {
            void* addr;
            char ip[INET6_ADDRSTRLEN];

            if (p->ai_family == AF_INET) {
                struct sockaddr_in* ipv4 = (struct sockaddr_in*)p->ai_addr;
                addr = &(ipv4->sin_addr);
            } else {
                struct sockaddr_in6* ipv6 = (struct sockaddr_in6*)p->ai_addr;
                addr = &(ipv6->sin6_addr);
            }

            inet_ntop(p->ai_family, addr, ip, sizeof(ip));
            snprintf(result + strlen(result), sizeof(result) - strlen(result), "%s\n", ip);
        }

        freeaddrinfo(res);
    }

    sendto(g_serverSocket, result, strlen(result), 0, (struct sockaddr*)&clientAddr, sizeof(clientAddr));
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s PortNumber\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serverAddr;
    struct sigaction sigAction;

    if ((g_serverSocket = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(atoi(argv[1]));

    if (bind(g_serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Bind failed");
        close(g_serverSocket);
        exit(EXIT_FAILURE);
    }

    // Signal handling setup
    sigAction.sa_handler = sig_io;
    sigemptyset(&sigAction.sa_mask);
    sigAction.sa_flags = 0;

    if (sigaction(SIGIO, &sigAction, NULL) == -1) {
        perror("Sigaction failed");
        close(g_serverSocket);
        exit(EXIT_FAILURE);
    }

    // Set socket options for signal-driven I/O
    int on = 1;
    ioctl(g_serverSocket, FIOASYNC, &on);
    fcntl(g_serverSocket, F_SETOWN, getpid());
    ioctl(g_serverSocket, FIONBIO, &on);

    printf("Server listening on port %d...\n", atoi(argv[1]));

    while (1) {
        sleep(1);
    }

    close(g_serverSocket);

    return 0;
}
