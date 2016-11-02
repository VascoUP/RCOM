CC=gcc
CFLAGS=-Wall -g

app: applicationlayer.o linklayer.o statistics.o interface.o
	$(CC) -o app applicationlayer.o linklayer.o statistics.o interface.o $(CFLAGS)
	make clean

applicationlayer.o: applicationlayer.c
	$(CC) -c applicationlayer.c $(CFLAGS)

linklayer.o: linklayer.c
	$(CC) -c linklayer.c $(CFLAGS)

statistics.o: statistics.c
	$(CC) -c statistics.c $(CFLAGS)

interface.o: interface.c
	$(CC) -c interface.c $(CFLAGS)
	
clean: 
	rm *o
