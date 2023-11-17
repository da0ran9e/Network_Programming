#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUFF_SIZE 1024
#define THREAD_MAX 5

typedef struct {
    char userID[BUFF_SIZE / 2 - 1];
    char password[BUFF_SIZE / 2 - 1];
    int status;
    int loginCount;
} UserInfo;

pthread_mutex_t lock_userArray;
UserInfo *userArray;
int userCount;

void initializeAccountsFromFile();
int userGetAccountIndex(const char *username);
int userCheckPassword(int accIndex, const char *input);
int userCheckLocked(int accIndex);
void userAddLoginCount(int accIndex);

void *connection_handler(void *client_socket) {
    int conn_sock = *(int *)client_socket;
    server_echo(conn_sock);
    close(conn_sock);

    return NULL;
}

void server_echo(int sockfd) {
    char buff[BUFF_SIZE + 1];
    int bytes_sent, bytes_received;

    char username[BUFF_SIZE / 2 - 1];
    char password[BUFF_SIZE / 2 - 1];

    int accIndex = -1;

    while (1) {
        bytes_received = recv(sockfd, buff, BUFF_SIZE, 0);

        if (bytes_received < 0)
            perror("\nError: ");
        else if (bytes_received == 0) {
            printf("Echo: Connection closed.\n");
            break;
        }
        buff[bytes_received] = '\0';

        sscanf(buff, "%s %s", username, password);
        int status = -1;

        accIndex = userGetAccountIndex(username);

        if (accIndex == -1) {
            status = STATUS_NO_ACCOUNT;
        } else {
            if (userCheckPassword(accIndex, password) == 1) {
                status = STATUS_LOGGED_IN;
            } else {
                status = STATUS_WRONG_CREDENTIAL;
                userAddLoginCount(accIndex);
            }

            if (userCheckLocked(accIndex) == 1) {
                status = STATUS_LOCKED;
            }
        }

        memset(buff, '\0', BUFF_SIZE);
        sprintf(buff, "%d", status);

        bytes_sent = send(sockfd, buff, bytes_received, 0);

        if (bytes_sent < 0)
            perror("\nError: ");
    }
}

void initializeAccountsFromFile() {
    // Your implementation here
}

int userGetAccountIndex(const char *username) {
    // Your implementation here
}

int userCheckPassword(int accIndex, const char *input) {
    // Your implementation here
}

int userCheckLocked(int accIndex) {
    pthread_mutex_lock(&lock_userArray);
    int over5 = userArray[accIndex].status;
    pthread_mutex_unlock(&lock_userArray);
    return over5;
}

void userAddLoginCount(int accIndex) {
    pthread_mutex_lock(&lock_userArray);

    userArray[accIndex].loginCount++;

    if (userArray[accIndex].loginCount >= 5 && userArray[accIndex].status != 1) {
        userArray[accIndex].status = 1;

        FILE *file = fopen("account.txt", "w");
        for (int i = 0; i < userCount; i++) {
            fprintf(file, "%s %s %d\n", userArray[i].userID, userArray[i].password, userArray[i].status);
        }
        fclose(file);
    }

    pthread_mutex_unlock(&lock_userArray);
}

int initialize_server(int port) {
    // Your implementation here
}

void run_server(int listen_sock) {
    // Your implementation here
}

void cleanup_server(int listen_sock) {
    close(listen_sock);
}

int main(int argc, char *argv[]) {
    // Your implementation here
}
