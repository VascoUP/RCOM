
#include "linklayer.h"
#include <string.h>

typedef struct {
    int fileDescriptor;
    int status;         //TRANSMITTER | RECEIVER
} applicationLayer;

typedef struct {
    FILE* fileDescriptor;
    char *file_name;
    unsigned int size;
    unsigned int read_size;
} fileInfo;

unsigned char* load_file(char *path, int *length, fileInfo *info) {

    FILE *fp;
    fp = fopen(path, "r");
    fseek(fp, 0, SEEK_END);
    *length = ftell(fp);
    rewind(fp);

    info->fileDescriptor = fp;
    info->size = *length;
    info->read_size = 0;

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

  fileInfo info;

  unsigned char *loaded_file = load_file(file, &file_size, &info);

  unsigned char *control = build_control_packet(2, file_size, file, &length);
  llwrite( fd, control, length );
  free(control);

  control = build_control_packet(3, file_size, file, &length);
  llwrite( fd, control, length );
  free(control);

  free(loaded_file);

  return 0;
}

int handler_read( unsigned char* data, int length, fileInfo *info ) {

    if( (unsigned int) data[0] == 2 ) {

      return 1;
    } else if( (unsigned int) data[0] == 3 ) {

      return 2;
    } else if( (unsigned int) data[0] == 1 ) {

      return 0;
    }

    /*
      Return 0 - Data packet
      Return 1 - Start packet
      Return 2 - End packet
    */
    return -1;
}

int receive_file( int fd ) {
    unsigned char *buffer = NULL;
    int length, type;
    fileInfo info;

    while( 1 ) {
        length = llread( fd, &buffer );
        type = handler_read(buffer, length, &info);
        if( type == 2 ) {
            printf("End\n");
            return 0;
        } else if( type == 1 ) {
            printf("Start\n");
        } else {
            printf("Normal %d", type);
        }
    }
    return -1;
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
    llread( info.fileDescriptor, &buf);
    llread( info.fileDescriptor, &buf);
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
