CC=gcc
CFLAGS=-Wall -g

download: FTPClient.o parser.o
	$(CC) -o download FTPClient.o parser.o $(CFLAGS)
	make clean

FTPClient.o: FTPClient.c
	$(CC) -c FTPClient.c $(CFLAGS)

parser.o: parser.c
	$(CC) -c parser.c $(CFLAGS)

clean:
	rm *o
