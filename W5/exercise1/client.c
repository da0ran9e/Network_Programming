#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAXLINE 1024

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <IP_Addr> <Port_Number>\n", argv[0]);
        exit(1);
    }

    int sockfd;
    char buffer[MAXLINE];
    struct sockaddr_in servaddr;

    char *IP_ADDR = argv[1];
    int PORT = atoi(argv[2]);

    // Create socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, IP_ADDR, &servaddr.sin_addr) <= 0) {
        perror("inet_pton");
        exit(1);
    }

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        perror("connect");
        exit(1);
    }

    while (1) {
        printf("Send to server: ");
        if (fgets(buffer, MAXLINE, stdin) == NULL) {
            break;
        }

        if (send(sockfd, buffer, strlen(buffer), 0) == -1) {
            perror("send");
            break;
        }

        ssize_t n = recv(sockfd, buffer, MAXLINE, 0);
        if (n <= 0) {
            perror("recv");
            break;
        }

        buffer[n] = '\0';
        printf("Received from server: %s", buffer);
    }

    close(sockfd);
    
    return 0;
}
