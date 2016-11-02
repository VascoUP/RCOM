
#include "linklayer.h"
#include "statistics.h"
#include "interface.h"

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <utime.h>
#include <time.h>

typedef struct {
    int fd;                     //file descriptor
    char *file_name;            //file name
    unsigned int size;          //file size
    unsigned int read_size;     //read size
    mode_t permission;	//permissions
    time_t m_time;				//date
} fileInfo;

typedef struct {
    unsigned int sequence_number;   //sequence number
    int status;                     //TRANSMITTER | RECEIVER
    fileInfo *info;                 //file information
} applicationLayer;

static int fileDescriptor = -1;             //file descriptor

//^C handler
void intHandler( ) {
    printf("\n\n----------\nCancelling\n----------\n\n");
    if( fileDescriptor != -1)
    	llclose(fileDescriptor);
    exit(0);
}

//Only used by the transmitter
unsigned char* load_file(char *path, fileInfo *info) {

    FILE *fp;
    if( (fp = fopen(path, "r")) == NULL ) {
        printf("load_file:: Couldnt open %s\n", path);
        return NULL;
    }

    fseek(fp, 0, SEEK_END);

    info->size = ftell(fp);
    info->read_size = 0;
    rewind(fp);

    unsigned char *data = malloc(sizeof(char) * (info->size + 1));
    fread(data, info->size, 1, fp);

    fclose(fp);

    //permissions and date
    int fd = open(path, O_RDONLY);
    struct stat filestat;
    if (fstat(fd, &filestat) < 0) {
        printf("load_file_info:: Failed to retrieve file info\n");
    }
    info->permission = filestat.st_mode;
    info->m_time = filestat.st_mtime;
    close(fd);

    return data;
}

int open_file( char* path , int role) {
	int fd;
	if(role == RECEIVER){
		int status = unlink(path);
    if( status != 0 )
        printf("Error removing the file %s\n", path);
        fd = open(path, O_WRONLY | O_CREAT | O_APPEND);
    } else
        fd = open(path, O_RDONLY);

    return fd;
}

int write_file( int fd, unsigned char* data, int length ) {
    int n = write(fd, data, length);
    return n;
}

void close_file( int fd, char* path , mode_t permission, time_t m_time) {
    close(fd);
    struct utimbuf new_time;
    new_time.modtime = m_time;
    utime(path, &new_time);
    chmod(path, /*S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH*/ permission);
}

int unpack_control_packet( unsigned char *data, unsigned int length, fileInfo *info ) {
    int t_index = 1;
    int l_index = 2;
    int v_index = 3;
    int size, i;
    unsigned char verify = 0;
    info->size = 0;
    info->permission = 0;

    while(t_index < length) {
        size = data[l_index];
        switch( data[t_index] ) {
        case 0: //file size
            for( i = 0; i < size; i++ )
                info->size = (info->size << 8) + (int) data[v_index + i];

            verify |= BIT(0);

            break;
        case 1: //file name
            info->file_name = malloc( (size+1) * sizeof(char) );

            memset(info->file_name, 0, size+1);
            memcpy(info->file_name, data + v_index , size);

            verify |= BIT(1);

            break;

        case 2:
        	for( i = 0; i < size; i++ )
                info->permission = (info->permission << 8) + (int) data[v_index + i];

            verify |= BIT(2);
            break;

        case 3:
        	for( i = 0; i < size; i++ )
                info->m_time = (info->m_time << 8) + (int) data[v_index + i];

            verify |= BIT(3);
            break;

        default:
            return -1;
        }

        t_index = v_index + data[l_index];
        l_index = t_index + 1;
        v_index = l_index + 1;

    }

    if( verify == 0xF )
    	return 0;

    printf("unpack_control_packet:: Unpacking was unsuccessfull\n");
    return -1;
}

unsigned char* build_control_packet( unsigned int control, unsigned int file_size, char *file_name, int *length, mode_t permissions, time_t m_time ) {
    const int fn_length = strlen(file_name);

    int fs_length = 0;
    unsigned int x = file_size;
    while (x != 0) {
    	x >>= 8;
    	fs_length++;
    }

    int p_length = 0;
    x= permissions;
    while (x != 0) {
    	x >>= 8;
    	p_length++;
    }

    int time_length = 0;
    x = m_time;
    while (x != 0) {
    	x >>= 8;
    	time_length++;
    }

    *length = 5 + fs_length + fn_length + 2 + p_length + 2 +  time_length;

    unsigned char *packet = malloc(*length * sizeof(unsigned char));
    if( packet == NULL ) {
        printf("build_control_packet:: Error allocating memory\n");
        return NULL;
    }

    packet[0] = control;
    packet[1] = 0;
    packet[2] = fs_length;

    int i;
    for (i = 0; i < fs_length; i++) {
    	packet[3+fs_length-1-i] = file_size >> (i*8);
    }

    packet[3+fs_length] = 1;
    packet[3+fs_length+1] = fn_length;
    memcpy(packet + 3 + fs_length + 2, file_name, fn_length);

    packet[3+fs_length+2+fn_length] = 2;
    packet[3+fs_length+2+fn_length+1] = p_length;
    for (i = 0; i < p_length; i++) {
    	packet[3 + fs_length + 2 + fn_length + 2 + p_length-1-i] = permissions >> (i*8);
    }

    packet[3 + fs_length + 2 + fn_length + 2 + p_length] = 3;
    packet[3 + fs_length + 2 + fn_length + 2 + p_length + 1 ] = time_length;
    for (i = 0; i < time_length; i++) {
    	packet[3 + fs_length + 2 + fn_length + 2 + p_length + 2 + time_length-1-i] = m_time >> (i*8);
    }

    return packet;
}

int unpack_data_packet( unsigned char** data, unsigned int sequence_number ) {
    int length = (int) ((*data)[2] << 8 | (*data)[3]);
    if( length <= 0 ) {
        printf("unpack_data_packet:: Packet length is invalid\n");
        return -1;
    }
    int sq_num = (int) (*data)[1];
    if( (int)sequence_number == sq_num + 1 || (sequence_number == 0 && sq_num == 255) ) {
        //Duplicated
        printf("unpack_data_packet:: Duplicated packet\n");
        incFrameRepeat();
        return -1;
    }

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

void send_file(applicationLayer app, char *file) {
    const int packet_max_size = ( info.maxLengthTrama - 7 ) / 2;
    int packet_size;

    unsigned char *loaded_file = load_file(file, app.info);
    if (loaded_file == NULL) {
    	printf("File does not exist\n");
    	return;
    }

    unsigned char *packet = build_control_packet(START_PACKET, app.info->size, file, &packet_size, app.info->permission, app.info->m_time);
    llwrite( fileDescriptor, packet, packet_size );

    //Send file -> 124 bytes at a time (128 total)
    //Last one might be less than 124 bytes
    unsigned int index, data_size;

    for( index = 0; index < app.info->size; index += packet_max_size ) {

        //Last packet might have to be shorter than the others
        data_size = (app.info->size - index < packet_max_size) ? app.info->size - index : packet_max_size;

        packet = malloc(data_size * sizeof(unsigned char));

        //Copy part of the file to the newly initialized packets
        memcpy(packet, loaded_file + index, data_size);

        if( (packet_size = build_data_packet( app.sequence_number, data_size, &packet)) == -1 ||
            llwrite( fileDescriptor, packet, packet_size ) == -1 ) {
            printf("send_file:: Error building or sending the packet\n");
            break;
        }

        app.info->read_size += data_size;
        printf("Sent: %d out of %d ( %d%% )\n",
                app.info->read_size, app.info->size, app.info->read_size * 100 / app.info->size);
        app.sequence_number >= 255 ? app.sequence_number = 0 : app.sequence_number++;

    }

    if( index < app.info->size )
        printf("Trying to send end packet\n");

    packet = build_control_packet(END_PACKET, app.info->size, file, &packet_size, app.info->permission, app.info->m_time);
    llwrite( fileDescriptor, packet, packet_size );

    free(loaded_file);
}

int handler_read( unsigned char* data, int length, applicationLayer* app ) {
    unsigned int type = (unsigned int) data[0];
    if( type == DATA_PACKET ) {
        int length = unpack_data_packet(&data, app->sequence_number);
        if( length != -1 ) {
            app->sequence_number >= 255 ? app->sequence_number = 0 : app->sequence_number++;
            app->info->read_size += length;
            write_file(app->info->fd, data, length);
        }

        return DATA_PACKET;       //Informs the function receive_file that it receives an data packet
    } else if( type == START_PACKET ) {
        //If it didn't receive the necessary information
        return unpack_control_packet( data, length, app->info ) == -1 ? -1 : START_PACKET;
    } else if( type == END_PACKET ) {

        fileInfo info2;
        //If it didn't receive the necessary information
        return ( unpack_control_packet( data, length, &info2 ) == -1 ||
            //If any information is wrong
            strcmp(info2.file_name, app->info->file_name) != 0 ||
            info2.size != app->info->size ||
            //It didn't read the start packet already
            app->info->file_name == NULL ) ? -1 : END_PACKET;
    }

    printf("handler_read:: Didnt receive any of the possible types of packets\n");
    return -1;
}

int receive_file( applicationLayer app ) {
    unsigned char *buffer = NULL;
    int length, type;

    app.info->file_name = NULL;
    app.info->read_size = 0;

    while( 1 ) { //Keeps reading until it receives an end packet

        if( (length = llread( fileDescriptor, &buffer )) == -1 )
            continue;
        type = handler_read(buffer, length, &app);
        if( type == DATA_PACKET ) {
            printf("Received %d out of %d ( %d%% )\n",
                    app.info->read_size, app.info->size,
                    app.info->read_size * 100 / app.info->size );
        } else if( type == START_PACKET ) {
            printf("receive_file:: Start packet\n");
            if( (app.info->fd = open_file( app.info->file_name , app.status)) == -1 ) {
                printf("receive_file:: Unable to open the file %s\n", app.info->file_name);
                break;
            }
        } else if( type == END_PACKET ) {
            printf("receive_file:: End packet\n");
            close_file( app.info->fd, app.info->file_name, app.info->permission, app.info->m_time );
            break;
        } else
        printf("receive_file:: Error\n");

    }

    if(app.info->file_name != NULL)
    free(app.info->file_name);

    return 0;
}

int main(int argc, char **argv) {

  	applicationLayer app;

    if( argc != 2 ) {
      printf("main:: Some arguments are missing\n");
      return -1;
    }

  	if( strcmp("receiver", argv[1])==0 ) {
  		app.status = RECEIVER;
      getInformationInterface(argv[1]);
  	}
  	else if( strcmp("transmitter", argv[1])==0 ) {
  		app.status = TRANSMITTER;
  		getInformationInterface(argv[1]);

  		FILE *fp;
  		if( (fp = fopen(info.fileName, "r")) == NULL ) {
  			printf("load_file:: Couldnt open %s\n", info.fileName);
  			return -1;
  		}

  		fclose(fp);
  	}
  	else
  		return -1;

	srand(time(NULL));
    signal(SIGINT, intHandler);

  	app.info = (fileInfo *) malloc( sizeof(fileInfo));
  	fileDescriptor = llopen( app.status );
  	app.sequence_number = 0;
  	if( fileDescriptor < 0 ) {
  		printf("main:: Error opening the serial port\n");
  		return -1;
  	}

  	if( app.status == TRANSMITTER ) {
  		send_file(app, info.fileName);
  	} else {
  		receive_file(app);
  	}

  	llclose(fileDescriptor);
  	if (app.status == TRANSMITTER) {
  		sleep(1);
  	}

	free(app.info);
  	statistics stat = getStatistics();

  	if( strcmp("transmitter", argv[1]) == 0){
  		printf("\n\n------------------Statistics Transmitter-----------------\n\n");
  		printf("Number of frames sent (START and END frames included): %d\n", stat.numFrameSend);
  		printf("Number of time-outs: %d\n", stat.numTimeOut);
  		printf("Number of rejects received: %d\n", stat.numREJReceive);
  	}

  	else {
  		printf("\n\n------------------Statistics Receiver-----------------\n\n");
  		printf("Number of frames received (START and END frames included): %d\n", stat.numFrameReceive);
  		printf("Number of frames repeated: %d\n", stat.numFrameRepeat);
  		printf("Number of rejects sent: %d\n", stat.numREJSend);
  	}

  	return 0;
}
