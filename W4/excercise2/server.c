#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_BUFF_SIZE 1024


void resolveDomainOrIP(const char *input, char *replyStr, char *errorStr) {
    int hasError = 0;

    struct addrinfo hints, *result, *rp;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;  // Chấp nhận cả IPv4 và IPv6

    int ret = getaddrinfo(input, NULL, &hints, &result);
    if (ret == 0) {
        for (rp = result; rp != NULL; rp = rp->ai_next) {
            if (rp->ai_family == AF_INET) {  // IPv4
                struct sockaddr_in *ipv4 = (struct sockaddr_in *)rp->ai_addr;
                strcat(replyStr, "Official IP: ");
                strcat(replyStr, inet_ntoa(ipv4->sin_addr))
                
            } else if (rp->ai_family == AF_INET6) {  // IPv6
                char ip6str[INET6_ADDRSTRLEN];
                struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)rp->ai_addr;
                inet_ntop(AF_INET6, &(ipv6->sin6_addr), ip6str, INET6_ADDRSTRLEN);
                strcat(replyStr, "Official IP: ");
                strcat(replyStr, ip6str)
                
            }
        }
        freeaddrinfo(result);
    } else {
        struct hostent *hostInfo;
        if (inet_pton(AF_INET, input, &(struct in_addr){}) == 1) {
            // Input is a valid IP address
            hostInfo = gethostbyaddr(input, strlen(input), AF_INET);
        } else {
            // Input is considered as a domain name
            hostInfo = gethostbyname(input);
        }

        if (hostInfo != NULL) {
            strcat(replyStr, "\nOfficial name: ");
            strcat(replyStr, hostInfo->h_name);
            
            strcat(replyStr, "\nAlias name:");
            
            char **alias = hostInfo->h_aliases;
            while (*alias != NULL) {
                strcat(replyStr, "\n");
                strcat(replyStr, *alias);
                alias++;
            }
        } else {
            strcat(replyStr, "Not found information\n");
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s PortNumber\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]); //string to integer

    int sockfd;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    // Create a socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation error: ");
        return 0;
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(port); //bind the socket to the given port

    // Bind the socket to the port
    if (bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Bind error: ");
        return 0;
    }

    printf("Server is running at port: %d\n", port);

    char buffer[MAX_BUFF_SIZE];
    char parameter[MAX_BUFF_SIZE];
    char reply[MAX_BUFF_SIZE];
    char errorStr[MAX_BUFF_SIZE];
    
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&clientAddr, &clientAddrLen);

        //check reveived data
        if (n < 0) {
            perror("Receive error");
            continue;
        }

        resolveDomainOrIP(buffer, replyStr, errorStr);

        // int alphaIndex = 0;
        // int digitIndex = 0;
        // int hasError = 0;

        // for (int i = 0; i < n; i++) {
        //     if (isalpha(buffer[i])) {
        //         alphabetStr[alphaIndex++] = buffer[i];
        //     } else if (isdigit(buffer[i])) {
        //         digitStr[digitIndex++] = buffer[i];
        //     } else {
        //         hasError = 1;
        //         break;
        //     }
        // }

        if (strlen(errorStr) > 0) {
            sendto(sockfd, errorStr, strlen(errorStr), 0, (struct sockaddr *)&clientAddr, clientAddrLen);
        } else {
            alphabetStr[alphaIndex] = '\0';
            digitStr[digitIndex] = '\0';

            sendto(sockfd, alphabetStr, strlen(alphabetStr), 0, (struct sockaddr *)&clientAddr, clientAddrLen);
            sendto(sockfd, digitStr, strlen(digitStr), 0, (struct sockaddr *)&clientAddr, clientAddrLen);
        }
    }

    close(sockfd);
    return 0;
}
