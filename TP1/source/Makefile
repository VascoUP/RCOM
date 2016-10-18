CC=gcc
CFLAGS=-Wall

app: applicationlayer.o linklayer.o
	$(CC) -o app applicationlayer.o linklayer.o $(CFLAGS)
	make clean

applicationlayer.o: applicationlayer.c
	$(CC) -c applicationlayer.c $(CFLAGS)

linklayer.o: linklayer.c
	$(CC) -c linklayer.c $(CFLAGS)
	
clean: 
	rm *o
