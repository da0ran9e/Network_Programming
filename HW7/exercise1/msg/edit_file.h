#ifndef EDIT_FILE_H
#define EDIT_FILE_H

void edit_file(char * content, int editmode, int key);
void delete_file(char * filename);
int create_server_directory();

extern const char directoryName[];

#endif