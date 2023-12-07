#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/stat.h>
#include <time.h>

#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

void deleteFile(char * file);
void saveFile(char * data);
void encrypt(char * data, int shift);
void decrypt(char * data, int shift);

#endif