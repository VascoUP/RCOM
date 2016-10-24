#include "linklayer.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

static linklayer ll;
static struct termios oldtio;

static int open_serial(int porta); 
static int close_serial(int fd);
static int write_serial(int fd, unsigned char msg[], int length);

// !!NOT FINISHED!!
int handleMessage(int length, unsigned char msg[]) {
	int i, type;
	unsigned char f1 = '\0', a = '\0', c = '\0', bcc1 = '\0', bcc2 = '\0';
	for( i = 0; i < length; i++ ) {
		if( f1 == '\0' ) {
			if( msg[i] == BYTE_FLAG ) {
				printf("FLAG: %x\n", msg[i]); 
				f1 = msg[i];
			}
		} else if( a == '\0' && i > 0 && msg[i-1] == f1 ) {
			if( msg[i] == BYTE_A ) {
				printf("FLAG: %x\n", msg[i]);
				a = msg[i];
			}
			else
				return ERR;
		} else if( i > 0 && msg[i-1] == a ) {
			switch( msg[i] ) {
				case BYTE_C_I:
				case BYTE_C_I2:
					type = TRAMA_I;
					break;
				case BYTE_C_SET:
					type = TRAMA_SET;
					break;
				case BYTE_C_DISC:
					type = TRAMA_DISC;
					break;
				case BYTE_C_UA:
					type = TRAMA_UA;
					break;
				case BYTE_C_RR:
				case 0x85:
					type = TRAMA_RR;
					break;
				case BYTE_C_REJ:
				case 0x81:
					type = TRAMA_REJ;
					break;
				default:
					return ERR;
			}
			printf("FLAG: %x\n", msg[i]);
			c = msg[i];
		} else if( i > 0 && msg[i-1] == c ) {
			if( (a ^ c) == msg[i] ) {
				printf("FLAG: %x\n", msg[i]);
				bcc1 = msg[i];
			}
			else
				return ERR;
		} else if( i > 0 && msg[i-1] == bcc1 ) {
			if( msg[i] == BYTE_FLAG ) {
				if( type != TRAMA_I )
					break;
				else
					return ERR;
			} else if( msg[i] != BYTE_FLAG ) {
				if( type != TRAMA_I )
					return ERR;
				/*
				else
					func_handler_info();
				*/
			}
		} else if( bcc1 != '\0' && msg[i] == bcc1 && type == TRAMA_I ) {
			printf("FLAG: %x\n", msg[i]);
			bcc2 = msg[i];
		} else if( i > 0 && msg[i-1] == bcc2 && type == TRAMA_I ) {
			return type;
		}
	}

	return 0;
}

int llopen(int porta, int status) {
    int fd;
	if((fd = open_serial(porta)) == -1) return -1; //Retornar logo se der erro

	/* Dependendo do status vai-se enviar (TRANSMITTER) ou receber (RECIEVER) uma mensagem */
	return fd;
}

int llclose(int fd) {

	/* Dependendo do status vai-se enviar (TRANSMITTER) ou receber (RECIEVER) uma mensagem */
	return close_serial(fd);

}

int llread(int fd, char * buffer) {
    /*
    ..
    */
    return 0; //return # characters read | -1 if error
}

int llwrite(int fd, char * buffer, int length) {
    /*
    ...
    */
    return 0; //return # characters written | -1 if error
}

static int open_serial(int porta) {
    int fd;
    struct termios newtio;

    if (porta != 0 || porta != 1) {
        return -1;
    }

    ll.baudrate = BAUDRATE;
    if (sprintf(ll.port, "/dev/ttyS%d", porta) < 0) {
        return -1;
    }
    
    fd = open(ll.port, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) {
        return -1;
    }

    if (tcgetattr(fd, &oldtio) == -1) {
        return -1;
    }
    
    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = ll.baudrate | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    newtio.c_lflag = 0;

    newtio.c_cc[VTIME] = 0;
    newtio.c_cc[VMIN] = 1;

    tcflush(fd, TCIOFLUSH);

    if (tcsetattr(fd, TCSANOW, &newtio) == -1) {
        return -1;
    }

    return fd;
}

static int close_serial(int fd) {
    if (tcsetattr(fd, TCSANOW, &oldtio) == -1) {
        return -1;
    }
    close(fd);
    return 0;
}

static int write_serial(int fd, unsigned char msg[], int length) {
    int nw = 0;
    nw = write(fd, msg, length);
    while (length > nw) {
        int n;
        n = write(fd, msg+nw, length-nw);
        if (n == -1) {
            return -1;
        }
        else if (!n) {
            break;
        }
        nw += n;
    }
    return 0;
}
