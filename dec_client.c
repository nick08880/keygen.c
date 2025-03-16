// Client for decryption server
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
        fprintf(stderr, "Usage: %s <ciphertext_file> <key_file> <port>\n", argv[0]);
        exit(1);
    }
    
    char *ciphertextFile = argv[1];
    char *keyFile = argv[2];
    int portNumber = atoi(argv[3]);
    char ciphertext[BUFFER_SIZE], key[BUFFER_SIZE], plaintext[BUFFER_SIZE];

    FILE *fp = fopen(ciphertextFile, "r");
    if (fp == NULL) {
        error("Error opening ciphertext file");
    }
    fgets(ciphertext, BUFFER_SIZE - 1, fp);
    fclose(fp);
    ciphertext[strcspn(ciphertext, "\n")] = '\0';  

    fp = fopen(keyFile, "r");
    if (fp == NULL) {
        error("Error opening key file");
    }
    fgets(key, BUFFER_SIZE - 1, fp);
    fclose(fp);
    key[strcspn(key, "\n")] = '\0';  

    if (strlen(key) < strlen(ciphertext)) {
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
        error("CLIENT: ERROR connecting to decryption server");
    }

    // Send client identifier
    send(socketFD, "dec_client\n", 11, 0);
    
    // Send ciphertext
    send(socketFD, ciphertext, strlen(ciphertext), 0);
    send(socketFD, "\n", 1, 0);
    usleep(50000); // Small delay
    
    // Send key
    send(socketFD, key, strlen(key), 0);
    send(socketFD, "\n", 1, 0);
    usleep(50000);

    printf("CLIENT: Ciphertext and key sent. Waiting for response...\n");
    fflush(stdout);

    memset(plaintext, '\0', BUFFER_SIZE);
    if (recv(socketFD, plaintext, BUFFER_SIZE - 1, 0) < 0) {
        error("CLIENT: ERROR receiving plaintext");
    }

    printf("CLIENT: Received plaintext: %s\n", plaintext);
    printf("%s\n", plaintext);

    close(socketFD);
    return 0;
}
