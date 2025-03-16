// Same structure as enc_server.c but will decrypt ciphertext back into plaintext using modulo 27
// Uses the same socket structure as enc_server
// Handling multiple clients with fork() implementation source: https://www.geeksforgeeks.org/fork-system-call/
// Bind socket to an IP address and port source: https://www.linuxhowtos.org/C_C++/socket.htm 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>

#define BUFFER_SIZE 1024
#define ALPHABET_SIZE 27  

void error(const char *msg) {
    perror(msg);
    exit(1);
}

void setupAddressStruct(struct sockaddr_in* address, int portNumber) {
    memset((char*) address, '\0', sizeof(*address)); 
    address->sin_family = AF_INET;
    address->sin_port = htons(portNumber);
    address->sin_addr.s_addr = INADDR_ANY;
}

void decryptText(char *ciphertext, char *key, char *plaintext) {
    printf("SERVER: Decrypting data...\n");
    fflush(stdout);

    size_t textLen = strlen(ciphertext);
    for (size_t i = 0; i < textLen; i++) {
        if (ciphertext[i] == '\0' || key[i] == '\0') break;
        int cipherIndex = (ciphertext[i] == ' ') ? 26 : ciphertext[i] - 'A';
        int keyIndex = (key[i] == ' ') ? 26 : key[i] - 'A';
        int plainIndex = (cipherIndex - keyIndex + ALPHABET_SIZE) % ALPHABET_SIZE; 
        plaintext[i] = (plainIndex == 26) ? ' ' : 'A' + plainIndex;
    }
    plaintext[textLen] = '\0';
    printf("SERVER: Decryption complete. Plaintext: %s\n", plaintext);
    fflush(stdout);
}

void handleClient(int connectionSocket) {
    char ciphertext[BUFFER_SIZE], key[BUFFER_SIZE], plaintext[BUFFER_SIZE];
    memset(ciphertext, '\0', BUFFER_SIZE);
    memset(key, '\0', BUFFER_SIZE);
    memset(plaintext, '\0', BUFFER_SIZE);

    ssize_t bytesReceived = recv(connectionSocket, ciphertext, BUFFER_SIZE - 1, 0);
    if (bytesReceived < 0) {
        error("SERVER: ERROR receiving ciphertext");
    }
    ciphertext[strcspn(ciphertext, "\n")] = '\0';
    printf("SERVER: Received ciphertext: %s\n", ciphertext);
    fflush(stdout);

    bytesReceived = recv(connectionSocket, key, BUFFER_SIZE - 1, 0);
    if (bytesReceived < 0) {
        error("SERVER: ERROR receiving key");
    }
    key[strcspn(key, "\n")] = '\0';
    printf("SERVER: Received key: %s\n", key);
    fflush(stdout);

    decryptText(ciphertext, key, plaintext);

    ssize_t bytesSent = send(connectionSocket, plaintext, strlen(plaintext), 0);
    if (bytesSent < 0) {
        error("SERVER: ERROR sending plaintext");
    }
    printf("SERVER: Sent plaintext: %s\n", plaintext);
    fflush(stdout);

    close(connectionSocket);
    exit(0);
}

int main(int argc, char *argv[]) {
    int listenSocket, connectionSocket, portNumber;
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t clientLen = sizeof(clientAddress);
    
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }
    
    portNumber = atoi(argv[1]);
    listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket < 0) {
        error("ERROR opening socket");
    }
    
    setupAddressStruct(&serverAddress, portNumber);
    
    printf("SERVER: Attempting to bind to port %d...\n", portNumber);
    fflush(stdout);
    if (bind(listenSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        error("ERROR on binding");
    }
    
    listen(listenSocket, 5);
    printf("Decryption server listening on port %d\n", portNumber);
    fflush(stdout);
    
    while (1) {
        connectionSocket = accept(listenSocket, (struct sockaddr*)&clientAddress, &clientLen);
        if (connectionSocket < 0) {
            error("ERROR on accept");
        }
        
        printf("SERVER: Connection accepted from a client!\n");
        fflush(stdout);

        pid_t pid = fork();
        if (pid == 0) {  
            close(listenSocket);
            handleClient(connectionSocket);
        } else if (pid > 0) {
            close(connectionSocket);
            while (waitpid(-1, NULL, WNOHANG) > 0);
        } else {
            error("ERROR on fork");
        }
    }
    close(listenSocket);
    return 0;
}