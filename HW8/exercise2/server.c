#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define MAX_BUFF_SIZE 1024
#define QSIZE 10

struct Datagram {
    void *dg_sa;
    socklen_t dg_salen;
    char dg_data[MAX_BUFF_SIZE];
};

static struct Datagram dg[QSIZE];
static int iget, iput, nqueue;

void resolve(FILE *out_stream, const char *input) {
    // Your implementation to resolve IP addresses or domain names
    // and write the result to the out_stream
}

void sig_io(int signo) {
    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof(cliaddr);
    int n;

    dg[iput].dg_salen = clilen;
    n = recvfrom(STDIN_FILENO, dg[iput].dg_data, MAX_BUFF_SIZE, 0,
                 (struct sockaddr *)&cliaddr, &dg[iput].dg_salen);

    if (n < 0) {
        perror("recvfrom error");
        exit(EXIT_FAILURE);
    }

    dg[iput].dg_sa = malloc(clilen);
    memcpy(dg[iput].dg_sa, &cliaddr, clilen);
    if (++iput >= QSIZE)
        iput = 0;

    if (iput == iget) {
        fprintf(stderr, "receive buffer overflow\n");
        exit(EXIT_FAILURE);
    }

    nqueue++;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s PortNumber\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int sockfd;
    struct sockaddr_in serverAddr;

    // Create socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set up server address struct
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(atoi(argv[1]));

    // Bind the socket to the specified port
    if (bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    int on = 1;
    sigset_t zeromask, newmask, oldmask;
    
    sigemptyset(&zeromask);
    sigemptyset(&oldmask);
    sigemptyset(&newmask);
    sigaddset(&newmask, SIGIO);

    signal(SIGIO, sig_io);
    fcntl(sockfd, F_SETOWN, getpid());
    ioctl(sockfd, FIOASYNC, &on);
    ioctl(sockfd, FIONBIO, &on);

    sigprocmask(SIG_BLOCK, &newmask, &oldmask);

    while (1) {
        while (nqueue == 0)
            sigsuspend(&zeromask);

        sigprocmask(SIG_SETMASK, &oldmask, NULL);

        char result[MAX_BUFF_SIZE];
        FILE *out_stream = fmemopen(result, MAX_BUFF_SIZE - 1, "w");
        resolve(out_stream, dg[iget].dg_data);
        fclose(out_stream);

        sendto(sockfd, result, MAX_BUFF_SIZE, 0,
               dg[iget].dg_sa, dg[iget].dg_salen);

        if (++iget >= QSIZE)
            iget = 0;

        sigprocmask(SIG_BLOCK, &newmask, &oldmask);
        nqueue--;
    }

    return 0;
}
