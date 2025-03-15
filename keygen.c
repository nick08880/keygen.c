#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ALLOWED_CHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZ " 
#define NUM_ALLOWED_CHARS 27  // 26 letters + space

int main(int argc, char *argv[]) {
    // Check if the correct number of arguments is provided
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <length_of_key>\n", argv[0]);
        return 1;
    }
    
    // Convert the provided argument to an integer
    int keySize = atoi(argv[1]);
    if (keySize <= 0) {
        fprintf(stderr, "Error: Key length must be a positive integer.\n");
        return 1;
    }
    
    // Initialize the random number generator
    srand(time(0));
    
    // Generate the key and print it
    for (int i = 0; i < keySize; i++) {
        putchar(ALLOWED_CHARS[rand() % NUM_ALLOWED_CHARS]);
    }
    putchar('\n');  // Ensure output ends with a newline
    
    return 0;
}
