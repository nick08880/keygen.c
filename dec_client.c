// Client for decryption server, from enc_client.c
// Uses same socket structure, but sends ciphertext instead of plaintext
// Source: Beej’s Guide to Network Programming - https://beej.us/guide/bgnet/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define BUFFER_SIZE 1024

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
        fprintf(stderr, "Usage: %s <ciphertext_file> <key_file> <port>\n", argv[0]);
        exit(1);
    }
    
    char *ciphertextFile = argv[1];
    char *keyFile = argv[2];
    int portNumber = atoi(argv[3]);
    char ciphertext[BUFFER_SIZE], key[BUFFER_SIZE], plaintext[BUFFER_SIZE];

    // Read ciphertext from file
    FILE *fp = fopen(ciphertextFile, "r");
    if (fp == NULL) {
        error("Error opening ciphertext file");
    }
    fgets(ciphertext, BUFFER_SIZE - 1, fp);
    fclose(fp);
    ciphertext[strcspn(ciphertext, "\n")] = '\0';  // Remove newline

    // Read key from file
    fp = fopen(keyFile, "r");
    if (fp == NULL) {
        error("Error opening key file");
    }
    fgets(key, BUFFER_SIZE - 1, fp);
    fclose(fp);
    key[strcspn(key, "\n")] = '\0';  // Remove newline

    // Check if key is long enough
    if (strlen(key) < strlen(ciphertext)) {
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
        error("CLIENT: ERROR connecting to decryption server");
    }

    // Send ciphertext
    send(socketFD, ciphertext, strlen(ciphertext), 0);
    send(socketFD, "\n", 1, 0);

    // Send key
    send(socketFD, key, strlen(key), 0);
    send(socketFD, "\n", 1, 0);

    printf("CLIENT: Ciphertext and key sent. Waiting for response...\n");
    fflush(stdout);

    // Receive plaintext
    memset(plaintext, '\0', BUFFER_SIZE);
    recv(socketFD, plaintext, BUFFER_SIZE - 1, 0);

    printf("CLIENT: Received plaintext: %s\n", plaintext);
    printf("%s\n", plaintext);

    close(socketFD);
    return 0;
}
