// handling multiple clients with fork() implementation source: https://www.geeksforgeeks.org/fork-system-call/
// Blind socket to an ip address and port source : https://www.linuxhowtos.org/C_C++/socket.htm 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUFFER_SIZE 1024
#define ALPHABET "ABCDEFGHIJKLMNOPQRSTUVWXYZ "  // Allowed characters
#define ALPHABET_SIZE 27  // 26 letters + space

// Error function=
void error(const char *msg) {
    perror(msg);
    exit(1);
}

// Set up the server address struct
void setupAddressStruct(struct sockaddr_in* address, int portNumber) {
    memset((char*) address, '\0', sizeof(*address)); 
    address->sin_family = AF_INET;
    address->sin_port = htons(portNumber);
    address->sin_addr.s_addr = INADDR_ANY;
}

// Encrypt plaintext (mod 27)
void encryptText(char *plaintext, char *key, char *ciphertext) {
    for (size_t i = 0; i < strlen(plaintext); i++) {
        int ptIndex = (plaintext[i] == ' ') ? 26 : plaintext[i] - 'A';
        int keyIndex = (key[i] == ' ') ? 26 : key[i] - 'A';
        int cipherIndex = (ptIndex + keyIndex) % ALPHABET_SIZE;
        ciphertext[i] = (cipherIndex == 26) ? ' ' : 'A' + cipherIndex;
    }
    ciphertext[strlen(plaintext)] = '\0';
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
    if (bind(listenSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        error("ERROR on binding");
    }
    
    listen(listenSocket, 5);
    printf("Encryption server listening on port %d\n", portNumber);
    
    while (1) {
        connectionSocket = accept(listenSocket, (struct sockaddr*)&clientAddress, &clientLen);
        if (connectionSocket < 0) {
            error("ERROR on accept");
        }
        
        pid_t pid = fork();
        if (pid == 0) {  // Child process
            close(listenSocket);
            char plaintext[BUFFER_SIZE], key[BUFFER_SIZE], ciphertext[BUFFER_SIZE];
            memset(plaintext, '\0', BUFFER_SIZE);
            memset(key, '\0', BUFFER_SIZE);
            memset(ciphertext, '\0', BUFFER_SIZE);
            
            read(connectionSocket, plaintext, BUFFER_SIZE - 1);
            read(connectionSocket, key, BUFFER_SIZE - 1);
            
            encryptText(plaintext, key, ciphertext);
            write(connectionSocket, ciphertext, strlen(ciphertext));
            
            close(connectionSocket);
            exit(0);
        } else {
            close(connectionSocket);
        }
    }
    close(listenSocket);
    return 0;
}
