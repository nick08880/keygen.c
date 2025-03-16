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

// Error function 
void error(const char *msg) {
    perror(msg);
    exit(1);
}

// Set up the server address struct
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

    // Read plaintext from file
    FILE *fp = fopen(plaintextFile, "r");
    if (fp == NULL) {
        error("Error opening plaintext file");
    }
    fgets(plaintext, BUFFER_SIZE - 1, fp);
    fclose(fp);
    plaintext[strcspn(plaintext, "\n")] = '\0';

    // Read key from file
    fp = fopen(keyFile, "r");
    if (fp == NULL) {
        error("Error opening key file");
    }
    fgets(key, BUFFER_SIZE - 1, fp);
    fclose(fp);
    key[strcspn(key, "\n")] = '\0';

    // Check if key is long enough
    if (strlen(key) < strlen(plaintext)) {
        fprintf(stderr, "Error: key is too short\n");
        exit(1);
    }

    // Set up the server connection
    int socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD < 0) {
        error("CLIENT: ERROR opening socket");
    }

    struct sockaddr_in serverAddress;
    setupAddressStruct(&serverAddress, portNumber, "localhost");

    if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        error("CLIENT: ERROR connecting");
    }

    // Send plaintext with newline separator
    char formattedPlaintext[BUFFER_SIZE];
    snprintf(formattedPlaintext, BUFFER_SIZE, "%.*s\n", BUFFER_SIZE - 2, plaintext);
    snprintf(formattedKey, BUFFER_SIZE, "%.*s\n", BUFFER_SIZE - 2, key);
    fflush(stdout);
    
    ssize_t bytesSent = write(socketFD, formattedPlaintext, strlen(formattedPlaintext));
    if (bytesSent < 0) {
        error("CLIENT: ERROR writing plaintext");
    }
    printf("CLIENT: Sent %ld bytes of formatted plaintext\n", bytesSent);
    fflush(stdout);

    // Send key with newline separator
    char formattedKey[BUFFER_SIZE];
    snprintf(formattedKey, sizeof(formattedKey) - 2, "%.*s", (int)(sizeof(formattedKey) - 3), key);
    strcat(formattedKey, "\n");  // Append newline

    printf("CLIENT: Sending formatted key: %s", formattedKey);
    fflush(stdout);
    
    bytesSent = write(socketFD, formattedKey, strlen(formattedKey));
    if (bytesSent < 0) {
        error("CLIENT: ERROR writing key");
    }
    printf("CLIENT: Sent %ld bytes of formatted key\n", bytesSent);
    fflush(stdout);

    // Receive ciphertext from server
    memset(ciphertext, '\0', BUFFER_SIZE);
    ssize_t bytesRead = read(socketFD, ciphertext, BUFFER_SIZE - 1);
    if (bytesRead < 0) {
        error("CLIENT: ERROR reading ciphertext");
    }
    printf("CLIENT: Received ciphertext: %s\n", ciphertext);
    fflush(stdout);

    // Print ciphertext to stdout
    printf("%s\n", ciphertext);

    // Close the socket
    close(socketFD);
    return 0;
}
