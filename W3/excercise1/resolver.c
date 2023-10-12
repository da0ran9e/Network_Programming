#include <stdio.h>
#include <netdb.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <domain_or_ip>\n", argv[0]);
        return 1;
    }

    char *input = argv[1];
    struct hostent *hostInfo;
    struct in_addr addr;

    if (inet_pton(AF_INET, input, &addr) == 1) {
        // It's a valid IP address
        hostInfo = gethostbyaddr(&addr, sizeof(struct in_addr), AF_INET);
        if (hostInfo != NULL) {
            printf("Official name: %s\n", hostInfo->h_name);
            printf("Alias name:\n");
            for (char **alias = hostInfo->h_aliases; *alias != NULL; alias++) {
                printf("%s\n", *alias);
            }
        } else {
            printf("Not found information\n");
        }
    } else {
        // It's a domain name
        hostInfo = gethostbyname(input);
        if (hostInfo != NULL) {
            printf("Official IP: %s\n", inet_ntoa(*((struct in_addr *)hostInfo->h_addr)));
            printf("Alias IP:\n");
            for (char **addr = hostInfo->h_addr_list; *addr != NULL; addr++) {
                printf("%s\n", inet_ntoa(*((struct in_addr *)*addr)));
            }
        } else {
            printf("Not found information\n");
        }
    }

    return 0;
}
