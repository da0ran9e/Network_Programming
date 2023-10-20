#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>

#define MAX_BUFF_SIZE 1024


int resolveDomainOrIP(const char *param, char output[]) {
    char tmp[MAX_BUFF_SIZE];
    int offset = 0;

    struct hostent *host_info;
    struct in_addr ipv4_addr;
    char **alias;

    if (inet_aton(param, &ipv4_addr)) {
        // Input is a valid IP address
        host_info = gethostbyaddr(&ipv4_addr, sizeof(struct in_addr), AF_INET);
        if (host_info == NULL) {
            strcat(tmp, "Not found information\n");
            return -1;
        }
        strcat(tmp, "Official name: ");
        strcat(tmp, host_info->h_name);
        strcat(tmp, "\n");
        
        alias = host_info->h_aliases;
        strcat(tmp,"Alias name:\n");
        while (*alias) {
            strcat(tmp,*alias);
            strcat(tmp, "\n");
            alias++;
        }
    } else {
        // Input is a domain name
        host_info = gethostbyname(param);
        if (host_info == NULL) {
            strcat(tmp,"Not found information\n");
            return -1;
        }
        strcat(tmp,"Official IP: ");
        strcat(tmp,inet_ntoa(*(struct in_addr *)host_info->h_addr));
        strcat(tmp,"\n");
    
        alias = host_info->h_addr_list;
        strcat(tmp,"Alias IP:\n");
       
        while (*alias) {
            strcat(tmp,inet_ntoa(*(struct in_addr *)*alias));
            strcat(tmp,"\n");
            alias++;
        }
    }

    for (int i=0; i<strlen(tmp); i++){
        output[offset++] = tmp[i];
    }

    return offset;
}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s PortNumber\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);

    int sockfd;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    // Create a socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation error");
        return 1;
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(port);

    // Bind the socket to the specified port
    if (bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Bind error");
        return 1;
    }

    printf("Server is running at port %d.\n", port);

    char buffer[MAX_BUFF_SIZE];
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&clientAddr, &clientAddrLen);

        if (n < 0) {
            perror("Receive error");
            continue;
        }

        int replyOffset = 0;

        char replyStr[MAX_BUFF_SIZE];
        char errorStr[] = "Error: Notfound information.";

        int hasError = 0;

        replyOffset = resolveDomainOrIP(buffer, replyStr);

        if (replyOffset == -1) hasError = 1;

        if (hasError) {
            sendto(sockfd, errorStr, strlen(errorStr), 0, (struct sockaddr *)&clientAddr, clientAddrLen);
        } else {
            replyStr[replyOffset] = '\0';

            sendto(sockfd, replyStr, strlen(replyStr), 0, (struct sockaddr *)&clientAddr, clientAddrLen);
        }
    }

    close(sockfd);
    return 0;
}
