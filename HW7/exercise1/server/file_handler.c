#include "include/caesar.h"
#include "include/file_handler.h"

void delete(char * file) {
	remove(file);
}

void save(char *data) {
    time_t currentTime;
    time(&currentTime);
    struct tm *localTime = localtime(&currentTime);

    // Format the time as "YYYY-MM-DD-hh-mm-ss"
    char filename[200] = "storage/temp";
    char formattedTime[20];
    strftime(formattedTime, sizeof(formattedTime), "%Y-%m-%d-%H-%M-%S", localTime);

    strcat(filename, formattedTime);
    strcat(filename, ".txt");

    if (data == NULL || filename == NULL) {
        fprintf(stderr, "Error: NULL pointer detected\n");
        return;
    }

    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    size_t dataSize = strlen(data);
    size_t elementsWritten = fwrite(data, 1, dataSize, file);

    if (elementsWritten != dataSize) {
        fprintf(stderr, "Error writing data to file\n");
        fclose(file);
        return;
    }
    fclose(file);

    printf("Data saved to %s\n", filename);
}

void encrypt(char * data, int shift) {
    Caesar(data, shift);
}

void decrypt(char * data, int shift) {
    Caesar(data, -shift);
}
