#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>

#define MAX_BUFF_SIZE 1024

void resolveDomainOrIP(const char *input) {
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

        int ipOffset = 0;
        int nameOffset = 0;
        int aliasOffset = 0;

        char ipStr[MAX_BUFF_SIZE];
        char nameStr[MAX_BUFF_SIZE];
        char aliasStr[MAX_BUFF_SIZE];
        char errorStr[] = "Error: Notfound information.";

        int hasError = 0;

        struct addrinfo hints, *result, *rp;
        memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_family = AF_UNSPEC;  // Chấp nhận cả IPv4 và IPv6

        int ret = getaddrinfo(input, NULL, &hints, &result);
        if (ret == 0) {
            for (rp = result; rp != NULL; rp = rp->ai_next) {
                if (rp->ai_family == AF_INET) {  // IPv4
                    struct sockaddr_in *ipv4 = (struct sockaddr_in *)rp->ai_addr;
                    char ip[] = inet_ntoa(ipv4->sin_addr);
                    for (int i = 0; i < strlen(ip); i++){
                        ipStr[ipOffset++] = ip[i];
                    }
                    
                } else if (rp->ai_family == AF_INET6) {  // IPv6
                    char ip6str[INET6_ADDRSTRLEN];
                    struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)rp->ai_addr;
                    inet_ntop(AF_INET6, &(ipv6->sin6_addr), ip6str, INET6_ADDRSTRLEN);
                    
                    char ip[] = ip6str;
                    for (int i = 0; i < strlen(ip); i++){
                        ipStr[ipOffset++] = ip[i];
                    }
                    
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
                char name[] = hostInfo->h_name;
                
                char aliasTemp[];
                char **alias = hostInfo->h_addr_list;
                while (*alias != NULL) {
                    strcat(aliasTemp, *alias);
                    strcat(aliasTemp, "\n");
                    alias++;
                }
                for (int i = 0; i < strlen(aliasTemp); i++){
                        aliasStr[aliasOffset++] = aliasTemp[i];
                    }
            } else {
                hasError = 1;
            }
        }

        if (hasError) {
            sendto(sockfd, errorStr, strlen(errorStr), 0, (struct sockaddr *)&clientAddr, clientAddrLen);
        } else {
            ipStr[ipOffset] = '\0';
            nameStr[nameOffset] = '\0';
            aliasStr[aliasOffset] = '\0';

            sendto(sockfd, ipStr, strlen(ipStr), 0, (struct sockaddr *)&clientAddr, clientAddrLen);
            sendto(sockfd, nameStr, strlen(nameStr), 0, (struct sockaddr *)&clientAddr, clientAddrLen);
            sendto(sockfd, aliasStr, strlen(aliasStr), 0, (struct sockaddr *)&clientAddr, clientAddrLen);
        }
    }

    close(sockfd);
    return 0;
}
