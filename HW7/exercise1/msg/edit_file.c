#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>

#include "edit_file.h"
#include "debug.h"

void delete_file(char * filename) {
	if (remove(filename) == 0) { 
		DEBUG_PRINTF("Deleted '%s' successfully.\n", filename); 
	} else { 
		DEBUG_PRINTF("Unable to delete '%s'\n", filename);
	}
}


// Function to perform Caesar cipher encryption
void encodeCaesar(char * text, int shift) {
	int i;
	for (i = 0; i < strlen(text); i++) {
		if (isalpha(text[i])) {
			char base = isupper(text[i]) ? 'A' : 'a';
			text[i] = (text[i] - base + shift) % 26 + base;
		}
		// Leave non-alphabetic characters unchanged
	}
}

// Function to perform Caesar cipher decryption
void decodeCaesar(char * text, int shift) {
	int i;
	for (i = 0; i < strlen(text); i++) {
		if (isalpha(text[i])) {
			char base = isupper(text[i]) ? 'A' : 'a';
			text[i] = (text[i] - base - shift + 26) % 26 + base;
		}
		// Leave non-alphabetic characters unchanged
	}
}


void edit_file(char * content, int editmode, int key) {
	switch (editmode) {
		case 0:
			printf("Encoding...\n");
			encodeCaesar(content, key);
			break;
		case 1:
			printf("Decoding...\n");
			decodeCaesar(content, key);
			break;
		default:
			DEBUG_PRINTF("Do nothing (%d) \n", editmode);
			// For client -1
	}
}

const char directoryName[] = "server_storage";

int create_server_directory() {
	// Check if the directory exists
	struct stat st;
	if (stat(directoryName, &st) == -1) {
		// If the directory does not exist, create it
		if (mkdir(directoryName, 0777) == 0) {
			printf("Directory '%s' created successfully.\n", directoryName);
			return 0;  // Success
		} else {
			perror("Error creating directory");
			return -1;  // Error
		}
	} else {
		printf("Directory '%s' exists.\n", directoryName);
		return 0;  // Already exists, treated as success
	}
}