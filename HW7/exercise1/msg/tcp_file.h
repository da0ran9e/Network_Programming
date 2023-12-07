#ifndef TCP_FILE_H
#define TCP_FILE_H

void send_file_to_socket(int sockfd, char * filename);
void recv_file_from_socket(int sockfd, char * filename, int editmode, int key);

#endif