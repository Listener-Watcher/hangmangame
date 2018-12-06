CC = gcc
EXECUTABLES = hangman_client hangmen_server

CFLAGS = -g
all:$(EXECUTABLES)
clean:
	rm -f core *.o $(EXECUTABLES) a.out *.a

hangman_client:hangman_client.c
	$(CC) $(CFLAGS) hangman_client.c -o hangman_client	

hangmen_server:hangmen_server.c
	$(CC) $(CFLAGS) hangmen_server.c -o hangmen_server
