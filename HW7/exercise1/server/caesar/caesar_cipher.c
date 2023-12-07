#include "../include/caesar.h"

// Function to encrypt data using the Caesar cipher
void Caesar(char *data, int shift) {
    while (*data) {
        if (isupper(*data)) {
            *data = ((*data - 'A' + shift) % 26) + 'A';
        }
        else if (islower(*data)) {
            *data = ((*data - 'a' + shift) % 26) + 'a';
        }
        data++;
    }
}
