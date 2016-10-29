
#include "linklayer.h"
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

typedef struct {
    int fileDescriptor;
    int status;         //TRANSMITTER | RECEIVER
} applicationLayer;

typedef struct {
    int fd;
    char *file_name;
    unsigned int size;
    unsigned int read_size;
	//int sequence_number;
} fileInfo;

//Only used by the transmitter
unsigned char* load_file(char *path, int *length, fileInfo *info) {

    FILE *fp;
    fp = fopen(path, "r");
    fseek(fp, 0, SEEK_END);
    *length = ftell(fp);
    rewind(fp);

    info->size = *length;
    info->read_size = 0;

    unsigned char *data = malloc(sizeof(char) * (*length + 1));
    fread(data, *length, 1, fp);

    fclose(fp);

    return data;
}

int open_file( char* path ) {
    remove(path);
    int fd = open(path, O_WRONLY | O_CREAT | O_EXCL | O_APPEND);
    return fd;
}

void write_file( int fd, unsigned char* data, int length ) {
    int n = write(fd, data, length);
    printf("N: %d\n", n);
}

void close_file( int fd ) {
    close(fd);
}

int unpack_control_packet( unsigned char *data, unsigned int length, fileInfo *info ) {
    int t_index = 1;
    int l_index = 2;
    int v_index = 3;
    int size, i;
    unsigned char verify = 0;

    while(t_index < length) {
        size = data[l_index];
        switch( data[t_index] ) {
            case 0: //file size
                for( i = 0; i < size; i++ )
                    info->size = (info->size << 8) + (int) data[v_index + i];
                printf("unpack_control_packet:: SIZE %d\n", info->size);

                verify |= BIT(0);

                break;
            case 1: //file name
                if( info->file_name == NULL ) {
                    info->file_name = malloc( size * sizeof(char) );
                    memset(info->file_name, 0, size);
                }
                memcpy(info->file_name, (char *) (data + v_index) , size);
                printf("unpack_control_packet:: FILE NAME %s size %d\n", info->file_name, size);

                verify |= BIT(1);

                break;
            default:
                return -1;
        }

        t_index = v_index + data[l_index];
        l_index = t_index + 1;
        v_index = l_index + 1;

    }

    if( verify == 0x3 )
        return 0;

    printf("unpack_control_packet:: Unpacking was unsuccessfull\n");
    return -1;
}

unsigned char* build_control_packet( unsigned int control, int file_size, char *file_name, int *length ) {
    const int fn_length = strlen(file_name);
	printf("FILE NAME: %s SIZE: %d\n", file_name, fn_length);
    *length = 7 + fn_length;

    unsigned char *packet = malloc(*length);
    if( packet == NULL ) {
		printf("build_control_packet:: Error reallocing memory\n");
        return NULL;
	}

    packet[0] = control;
    packet[1] = 0;
    packet[2] = 2;
    packet[4] = file_size;
    packet[3] = file_size >> 8;
    printf("Size: %d - %d\n", file_size >> 8, file_size);
    packet[5] = 1;
    packet[6] = fn_length;
    memcpy(packet + 7, file_name, fn_length);

    return packet;
}

int unpack_data_packet( unsigned char** data ) {
	int length = (int) ((*data)[2] << 8 | (*data)[3]);
	//int sequence_number = (int) (*data)[1];

	memmove(*data, *data + 4, length);
	unsigned char* tmp = realloc(*data, length * sizeof(unsigned char));
	if( tmp == NULL ) {
		printf("unpack_data_packet:: Error reallocing memory\n");
		return -1;
	}
	*data = tmp;

	return length;
}

int build_data_packet( unsigned int sequenceNumber, unsigned int nBytes, unsigned char **data ) {
    unsigned char* tmp = realloc(*data, nBytes + 4 * sizeof(unsigned char));
    if( tmp == NULL ) {
        printf("build_data_packet:: Error reallocing memory\n");
        return -1;
    }
    *data = tmp;
    memmove(*data + 4, *data, nBytes);

    (*data)[0] = DATA_PACKET;
    (*data)[1] = sequenceNumber;
    (*data)[2] = (nBytes >> 8);
    (*data)[3] = nBytes;

    return nBytes + 4;
}

void send_file(int fd, char *file) {
    int file_size;
    int length;

    fileInfo info;
	  //info.sequence_number = 1;

    unsigned char *loaded_file = load_file(file, &file_size, &info);

    unsigned char *control = build_control_packet(START_PACKET, file_size, file, &length);
  	printf("send_file:: Send start packet (control %d)\n", (int) control[0]);
    llwrite( fd, control, length );
    free(control);

  	//Send file -> 124 bytes at a time (128 total)
  	unsigned int index, data_size, sent;
  	int packet_size;
  	unsigned char *packet;

    for( index = 0; index < file_size; index += 124 ) {

  		//Last packet might have to be shorter than the others
  		data_size = (file_size - index < 124) ? file_size - index : 124;

  		packet = malloc(data_size * sizeof(unsigned char));

  		//Copy part of the file to the newly initialized packets
  		memcpy(packet, loaded_file + index, data_size);

  		if( (packet_size = build_data_packet( 1, data_size, &packet)) == -1 ) {
  			free(packet);
  			break;
  		}

  		llwrite( fd, packet, packet_size );
  		info.read_size += data_size;
  		sent = info.read_size * 100;
  		sent /= info.size;
  		printf("%d - Sent: %d out of %d ( %d )\n", index + 1, info.read_size, info.size, sent);

  		free(packet);
    }

    control = build_control_packet(END_PACKET, file_size, file, &length);
	  printf("send_file:: Send end packet (control %d)\n", (int) control[0]);
    llwrite( fd, control, length );
    free(control);

    free(loaded_file);
}

int handler_read( unsigned char* data, int length, fileInfo *info, unsigned int start ) {

    if( (unsigned int) data[0] == DATA_PACKET ) {
        if( !start ) {
			      printf("handler_read:: Havent received start packet\n");
            return -1;
		    }

		int length = unpack_data_packet(&data);
    info->read_size += length;
		write_file(info->fd, data, length);

        return DATA_PACKET;       //Informa funcao receive_file que recebeu uma data packet
    } else if( (unsigned int) data[0] == START_PACKET ) {
        if( start ) {
			printf("handler_read:: Already received start packet\n");
            return -1;
		}
        //Se nao tiver recebido todas as informacoes necessarias
        return unpack_control_packet( data, length, info ) == -1 ? -1 : START_PACKET;
    } else if( (unsigned int) data[0] == END_PACKET ) {
        if( !start ) {
			printf("handler_read:: Havent received start packet\n");
            return -1;
		}

        fileInfo info2;
            //Se nao tiver recebido todas as informacoes necessarias
        return ( unpack_control_packet( data, length, &info2 ) == -1 ||
            //Se alguma das informacoes estiver errada
            strcmp(info2.file_name, info->file_name) != 0 ||
            info2.size != info->size ||
            //Ainda nao leu o start packet
            info->file_name == NULL ) ?
            -1 : END_PACKET;
    }

	printf("handler_read:: Didnt receive any of the possible types of packets\n");
    return -1;
}

int receive_file( int fd ) {
    unsigned char *buffer = NULL;
    unsigned int start = 0;     //0 - hasnt received start packet; 1 - it has already received the start packet
    int length, type;
    fileInfo info;
    info.file_name = NULL;
    info.read_size = 0;
    int i = 1;

    while( 1 ) { //Keeps reading until it receives an end packet

        length = llread( fd, &buffer );
        type = handler_read(buffer, length, &info, start);
        if( type == DATA_PACKET ) {
            printf("receive_file:: %d - Received %d out of %d ( %d%% )\n", i++, info.read_size, info.size, info.read_size * 100 / info.size );
        } else if( type == START_PACKET ) {
            start = 1;
            printf("receive_file:: Start\n");
            info.fd = open_file( info.file_name );
        } else if( type == END_PACKET ) {
            printf("receive_file:: End\n");
            close_file( info.fd );
            break;
        } else
            printf("receive_file:: Error\n");

    }

    if(info.file_name != NULL)
        free(info.file_name);

    return 0;
}

int main(int argc, char **argv) {

    applicationLayer al;
    int port;
    char file[MAX_LEN];

    if( strcmp("receiver", argv[1])==0 ) {
        if( argc != 3 ) {
            printf("main:: Falta argumentos\n");
            return -1;
        }

        al.status = RECEIVER;
    }
    else if( strcmp("transmitter", argv[1])==0 ) {
        if( argc != 4 ) {
            printf("main:: Falta argumentos\n");
            return -1;
        }

        al.status = TRANSMITTER;
    strcpy(file, argv[3]);
    }
    else
        return -1;

    if( strcmp("/dev/ttyS0", argv[2])==0 )
        port = 0;
    else if( strcmp("/dev/ttyS1", argv[2])==0 )
        port = 1;
    else {
        printf("main:: Porta nao existente\n");
        return -1;
    }

    al.fileDescriptor = llopen( port, al.status );
    if( al.fileDescriptor < 0 ) {
        printf("main:: Erro ao abrir a porta de série\n");
        return -1;
    }

    if( al.status == TRANSMITTER ) {
        send_file(al.fileDescriptor, file);
    } else {
        receive_file(al.fileDescriptor);
    }

    int count = 0;
    do {
        if( llclose(al.fileDescriptor) == 0 )
            break;
        count++;
    } while( count < 3 );

    if( count == 3 )
        return -1;
    return 0;
}
