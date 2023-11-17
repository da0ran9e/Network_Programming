#ifndef READ_ACCOUNT_H
#define READ_ACCOUNT_H

// Define the structure for user information
typedef struct UserInfo_s {
	char userID[100];
	char password[100];
	int status;
	int loginCount;
} UserInfo;

// Return pointer
void initializeAccountsFromFile();

// For server
int userGetAccountIndex(char * username);
int userCheckPassword(int accIndex, char * input);
int userCheckLocked(int accIndex);
void userAddLoginCount(int accIndex);


#endif