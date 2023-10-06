#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <arpa/inet.h>

void resolveDomainOrIP(const char *param) {
    struct hostent *host_info;
    struct in_addr ipv4_addr;
    char **alias;

    if (inet_aton(param, &ipv4_addr)) {
        // Input is a valid IP address
        host_info = gethostbyaddr(&ipv4_addr, sizeof(struct in_addr), AF_INET);
        if (host_info == NULL) {
            printf("Not found information\n");
            return;
        }
        printf("Official name: %s\n", host_info->h_name);
        alias = host_info->h_aliases;
        printf("Alias name:\n");
        while (*alias) {
            printf("%s\n", *alias);
            alias++;
        }
    } else {
        // Input is a domain name
        host_info = gethostbyname(param);
        if (host_info == NULL) {
            printf("Not found information\n");
            return;
        }
        printf("Official IP: %s\n", inet_ntoa(*(struct in_addr *)host_info->h_addr));
        alias = host_info->h_addr_list;
        printf("Alias IP:\n");
        while (*alias) {
            printf("%s\n", inet_ntoa(*(struct in_addr *)*alias));
            alias++;
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <domain/IP>\n", argv[0]);
        return 1;
    }

    char *param = argv[1];
    resolveDomainOrIP(param);

    return 0;
}
