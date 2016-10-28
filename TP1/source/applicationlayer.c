
#include "linklayer.h"
#include <string.h>

typedef struct {
    int fileDescriptor;
    int status;         //TRANSMITTER | RECEIVER
} applicationLayer;

unsigned char* load_file(char *path, int *length) {
    
    FILE *fp;
    fp = fopen(path, "r");

    fseek(fp, 0, SEEK_END);
    *length = ftell(fp);
    rewind(fp);

    unsigned char *data = malloc(sizeof(char) * (*length + 1));
    fread(data, *length, 1, fp);

    return data;
}

unsigned char* build_control_packet( unsigned int control, int *t, int *l, unsigned char **v, int length ) {
    int nBytes = 1 + 2 * length;
    int i;
    for( i = 0; i < length; i++ ) {
	nBytes += l[i];
    }

    unsigned char *packet = malloc(nBytes);
    if( packet == NULL )
	return NULL;

    packet[0] = control;
    
    int j = 1;
    for( i = 0; i < length; i++ ) {
	packet[j++] = t[i];
	packet[j++] = l[i];
	memcpy(packet+j, v[i], l[i]); 
	j += l[i];
    }

    return packet;
}

int build_data_packet( unsigned int control, unsigned int sequenceNumber, unsigned int nBytes, unsigned char **data ) {
    unsigned char* tmp = realloc(*data, nBytes + 4 * sizeof(unsigned char));
    if( tmp == NULL )
	return -1;
    *data = tmp;
    memmove(*data + 4, *data, nBytes);

    (*data)[0] = control;
    (*data)[1] = sequenceNumber;
    (*data)[2] = (unsigned char) (nBytes >> 8);
    (*data)[3] = (unsigned char) nBytes;

    return 0;
}

int send_file(char *file) {

    int file_size;
    load_file(file, &file_size);

    printf("%d\n", file_size);

    //unsigned char *data = build_control_packet(2, t, l, v); 

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
    char file[MAX_LEN];

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
    	strcpy(file, argv[3]);
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

    if( info.status == TRANSMITTER ) {
	send_file(file);
    }

/*    char *msg = (char *) malloc(13 * sizeof(char));
    if( info.status == TRANSMITTER ) {
        strcpy(msg, "TESt4R U2e*'");
        llwrite(info.fileDescriptor, (unsigned char *) msg, 12);
    } else {
        int n = llread(info.fileDescriptor, (unsigned char **) &msg);
        int a;
        for( a = 0; a < n; a++ )
            printf("%c", msg[a]);
        printf("\n");
    }
*/
    int count = 0;
    do {
        if( llclose(info.fileDescriptor) == 0 )
            break;
        count++;
    } while( count < 3 );

    //free(msg);

    if( count == 3 )
        return -1;
    return 0;
}
