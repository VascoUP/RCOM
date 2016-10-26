
#include "linklayer.h"
#include <string.h>

typedef struct {
    int fileDescriptor;
    int status;         //TRANSMITTER | RECEIVER
} applicationLayer;

int handler_write( /* args */ ) {
	/*
		Nao sei se e preciso, so usado se no futuro precisarmos
	*/
	return 0;
}

int send_file() {
	/*
		Chama as funcoes llwrite com as tramas I que contém a info da imagem
	*/


	return 0;
}

int handler_read( /* args */ ) {
	/*
		Handler da trama I recebida ao ler
			- Tem de detetar duplicados, ou erros
	*/
	return 0;
}

int receive_file( /* args */ ) {
	/*
		Constroi uma imagem com as info das tramas I recebidas
	*/
	return 0;
}

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

	char *msg = (char *) malloc(13 * sizeof(char));
	if( info.status == TRANSMITTER ) {
		strcpy(msg, "oooooooooooo");
		/*if( */llwrite(info.fileDescriptor, (unsigned char *) msg, 12) /*< 0 ) llclose()*/;
	} else {
        printf("Reading app\n");
		int n = llread(info.fileDescriptor, (unsigned char *) msg);
		int a;
		for( a = 0; a < n; a++ )
			printf("%c", msg[a]);
		printf("\nEnd message\n");
	}
    free(msg);

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
