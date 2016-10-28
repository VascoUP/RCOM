
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

unsigned char* build_control_packet( unsigned int control, int file_size, char *file_name, int *length ) {
  const int fn_length = strlen(file_name);
  *length = 7 + fn_length;

  unsigned char *packet = malloc(*length);
  if( packet == NULL )
    return NULL;

  packet[0] = control;
  packet[1] = 0;
  packet[2] = 2;
  packet[3] = (unsigned char) file_size >> 8;
  packet[4] = (unsigned char) file_size;
  packet[5] = 1;
  packet[6] = fn_length;
  memcpy(packet + 7, file_name, fn_length);

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

int send_file(int fd, char *file) {
  int file_size;
  int length;
  load_file(file, &file_size);

  printf("%d\n", file_size);

  unsigned char *control = build_control_packet(2, file_size, file, &length);
  llwrite( fd, control, length );

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
     send_file(info.fileDescriptor, file);
  } else {
    unsigned char* buf = NULL;
    int n = llread( info.fileDescriptor, &buf);
    int i;
    for( i = 0; i < n; i++ )
      printf("0x%02x\n", buf[i]);
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
