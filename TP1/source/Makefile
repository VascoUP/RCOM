CC=gcc
CFLAGS=-Wall

app: applicationlayer.o linklayer.o statistics.o
	$(CC) -o app applicationlayer.o linklayer.o statistics.o $(CFLAGS)
	make clean

applicationlayer.o: applicationlayer.c
	$(CC) -c applicationlayer.c $(CFLAGS)

linklayer.o: linklayer.c
	$(CC) -c linklayer.c $(CFLAGS)

statistics.o: statistics.c
	$(CC) -c statistics.c $(CFLAGS)
	
clean: 
	rm *o
