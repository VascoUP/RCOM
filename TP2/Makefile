CC=gcc
CFLAGS=-Wall -g

client: FTPClient.o parser.o
	$(CC) -o client FTPClient.o parser.o $(CFLAGS)
	make clean

FTPClient.o: FTPClient.c
	$(CC) -c FTPClient.c $(CFLAGS)

parser.o: parser.c
	$(CC) -c parser.c $(CFLAGS)
	
clean: 
	rm *o
