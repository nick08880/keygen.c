CC = gcc
CFLAGS = -Wall -std=gnu99

all: enc_server enc_client keygen dec_server dec_client

enc_server: enc_server.c
	$(CC) $(CFLAGS) -o enc_server enc_server.c

enc_client: enc_client.c
	$(CC) $(CFLAGS) -o enc_client enc_client.c

keygen: keygen.c
	$(CC) $(CFLAGS) -o keygen keygen.c

dec_server: dec_server.c
	$(CC) $(CFLAGS) -o dec_server dec_server.c

dec_client: dec_client.c
	$(CC) $(CFLAGS) -o dec_client dec_client.c

clean:
	rm -f enc_server enc_client keygen dec_server dec_client
