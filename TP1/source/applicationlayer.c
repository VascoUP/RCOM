
#include "linklayer.h"
#include <string.h>
#include <stdio.h>

typedef struct {
    int fileDescriptor;
    int status;         //TRANSMITTER | RECEIVER
} applicationLayer;

int main(int argc, char **argv) {

	applicationLayer info;
	int port;

	if( strcmp("receiver", argv[1])==0 ) { 
		if( argc != 3 ) {
			printf("1 - Falta argumentos\n");
			return -1;
		}

		info.status = RECEIVER;

	}
	else if( strcmp("transmitter", argv[1])==0 ) { 
		if( argc != 4 ) {
			printf("2 - Falta argumentos\n");
			return -1;
		}

		info.status = TRANSMITTER;

	}
	else
		return -1;

	if( strcmp("/dev/ttyS0", argv[2])==0 )
		port = 0;
	else if( strcmp("/dev/ttyS1", argv[2])==0 )
		port = 1;
	else {
		printf("Porta nao existente\n");
		return -1;
	}

	info.fileDescriptor = llopen( port, info.status );
	if( info.fileDescriptor < 0 ) {
		printf("Erro ao abrir a porta de série\n");
		return -1;
	}

	int count = 0;
	do {
		if( llclose(info.fileDescriptor) == 0 )
			break;
		count++;
	} while( count < 3 );

	if( count == 3 )
		return -1;
	return 0;
}
