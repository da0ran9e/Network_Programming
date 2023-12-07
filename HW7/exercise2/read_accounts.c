#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "read_accounts.h"

UserInfo * userArray;
int userCount;


int userGetAccountIndex(char * username) {
	for(int i = 0; i < userCount; i++) {
					// printf("%s %s\n", userArray[i].userID, username);
		if (strcmp(userArray[i].userID, username) == 0) {
			return i;
		}
	}
	return -1;
}

int userCheckPassword(int accIndex, char * input) {
	return strcmp(userArray[accIndex].password, input) == 0;
}

int userCheckLocked(int accIndex) {
	int over5 = userArray[accIndex].status;
	return over5;
}

void userResetLoginCount(int accIndex) {
	userArray[accIndex].loginCount = 0;
}

void userAddLoginCount(int accIndex) {	// i = accountIndex
	userArray[accIndex].loginCount++;

	// If over 5
	if (userArray[accIndex].loginCount >= 5 && userArray[accIndex].status != 1) {
		userArray[accIndex].status = 1;

		// Save immediately
		FILE * file = fopen("account.txt", "w");
		for(int i = 0; i < userCount; i++) {
			fprintf(file, "%s %s %d\n", userArray[i].userID, userArray[i].password, userArray[i].status);
		}
		fclose(file);
	}
}


// Return pointer
void initializeAccountsFromFile() {	// USE ONLY ONCE
	FILE * file = fopen("account.txt", "r");
	userArray = NULL;
	userCount = 0;

	if (file == NULL) {
		perror("Error opening account.txt\n");
		exit(1);
	}
	
	// Count the number of lines in the file
	int lines = 1;
	char c;
	while ((c = fgetc(file)) != EOF) {
		if (c == '\n') {
			lines++;
		}
	}

	// Allocate memory for the array of UserInfo into the pointer
	userArray = (UserInfo*) malloc(lines * sizeof(UserInfo));

	if (userArray == NULL) {
		perror("Memory allocation error");
		fclose(file);
		exit(1);
	}

	// Reset file position to the beginning
	fseek(file, 0, SEEK_SET);

	// Read user accounts into the array
	int info_read = 0;
	for (int i = 0; i < lines; i++) {
		info_read = fscanf(file, "%s %s %d", userArray[i].userID, userArray[i].password, &userArray[i].status);
		if (info_read != 3) {
			printf("Err: Error reading line %d\n", i + 1);
			continue;
		}
		// Add to global
		userCount++;
	}

	fclose(file);
	printf("Accounts: %d\n", userCount);
}