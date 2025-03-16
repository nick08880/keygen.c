// Source for using snprintf():  https://linux.die.net/man/3/snprintf
// Source client-server setup: https://linux.die.net/man/3/snprintf
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define BUFFER_SIZE 1024 

char formattedPlaintext[BUFFER_SIZE];
char formattedKey[BUFFER_SIZE];

#define BUFFER_SIZE 1024

void error(const char *msg) {
    perror(msg);
    exit(1);
}

void setupAddressStruct(struct sockaddr_in* address, int portNumber, char* hostname) {
    memset((char*) address, '\0', sizeof(*address));
    address->sin_family = AF_INET;
    address->sin_port = htons(portNumber);
    struct hostent* hostInfo = gethostbyname(hostname);
    if (hostInfo == NULL) {
        fprintf(stderr, "CLIENT: ERROR, no such host\n");
        exit(1);
    }
    memcpy((char*) &address->sin_addr.s_addr, hostInfo->h_addr_list[0], hostInfo->h_length);
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <plaintext_file> <key_file> <port>\n", argv[0]);
        exit(1);
    }
    
    char *plaintextFile = argv[1];
    char *keyFile = argv[2];
    int portNumber = atoi(argv[3]);
    char plaintext[BUFFER_SIZE], key[BUFFER_SIZE], ciphertext[BUFFER_SIZE];

    FILE *fp = fopen(plaintextFile, "r");
    if (fp == NULL) {
        error("Error opening plaintext file");
    }
    fgets(plaintext, BUFFER_SIZE - 1, fp);
    fclose(fp);
    plaintext[strcspn(plaintext, "\n")] = '\0';

    fp = fopen(keyFile, "r");
    if (fp == NULL) {
        error("Error opening key file");
    }
    fgets(key, BUFFER_SIZE - 1, fp);
    fclose(fp);
    key[strcspn(key, "\n")] = '\0';

    if (strlen(key) < strlen(plaintext)) {
        fprintf(stderr, "Error: key is too short\n");
        exit(1);
    }

    int socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD < 0) {
        error("CLIENT: ERROR opening socket");
    }

    struct sockaddr_in serverAddress;
    setupAddressStruct(&serverAddress, portNumber, "localhost");

    if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        error("CLIENT: ERROR connecting");
    }

    char formattedPlaintext[BUFFER_SIZE];
    strncpy(formattedPlaintext, plaintext, BUFFER_SIZE - 2); // Copy safely
    formattedPlaintext[BUFFER_SIZE - 2] = '\0';  // Ensure null termination
    strcat(formattedPlaintext, "\n");  // Append newline safely
    formattedPlaintext[BUFFER_SIZE - 1] = '\0'; // Ensure null termination
    strncpy(formattedKey, key, BUFFER_SIZE - 2);  // Copy safely
    formattedKey[BUFFER_SIZE - 2] = '\0';  // Ensure null termination
    strcat(formattedKey, "\n");  // Append newline safely
    ssize_t bytesSent = write(socketFD, formattedPlaintext, strlen(formattedPlaintext));
    if (bytesSent < 0) {
        error("CLIENT: ERROR writing plaintext");
    }

    char formattedKey[BUFFER_SIZE];
    strncpy(formattedKey, key, BUFFER_SIZE - 2);  // Copy key safely
    formattedKey[BUFFER_SIZE - 2] = '\0';  // Ensure null termination
    strcat(formattedKey, "\n");  // Append newline safely
    bytesSent = write(socketFD, formattedKey, strlen(formattedKey));
    if (bytesSent < 0) {
        error("CLIENT: ERROR writing key");
    }

    memset(ciphertext, '\0', BUFFER_SIZE);
    ssize_t bytesRead = read(socketFD, ciphertext, BUFFER_SIZE - 1);
    if (bytesRead < 0) {
        error("CLIENT: ERROR reading ciphertext");
    }

    printf("%s\n", ciphertext);
    close(socketFD);
    return 0;
}
