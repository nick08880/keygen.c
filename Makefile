CC = gcc
CFLAGS = -Wall -std=gnu99

all: enc_server enc_client keygen  # Excluding dec_server and dec_client for now

enc_server: enc_server.c
	$(CC) $(CFLAGS) -o enc_server enc_server.c

enc_client: enc_client.c
	$(CC) $(CFLAGS) -o enc_client enc_client.c

keygen: keygen.c
	$(CC) $(CFLAGS) -o keygen keygen.c

clean:
	rm -f enc_server enc_client keygen  # Excluding dec_server and dec_client
