#include "linklayer.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>

static linklayer ll;
static struct termios oldtio;
static int flag = 1;
static int counter = 0;

static int open_serial(int porta); 
static int close_serial(int fd);
static int write_serial(int fd, unsigned char msg[], int length);
static int read_serial(int fd, unsigned char *buf);

void atende() {
    printf("alarme #%d\n", ++counter);
    flag = 1;
}

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
	if((fd = open_serial(porta)) == -1)  {
		printf("Erro open_serial\n");
		return -1; //Retornar logo se der erro
	}

	struct sigaction actionAlarm;
	actionAlarm.sa_handler = atende;
	sigemptyset(&actionAlarm.sa_mask);
	actionAlarm.sa_flags = 0;

	if(sigaction(SIGALRM, &actionAlarm, NULL) < 0){
		fprintf(stderr, "Error activating the alarm\n");
		return -1;
	}

	unsigned char buffer[MAX_LEN];

	if( status == TRANSMITTER ) {
		unsigned char set[5];
		set[0] = BYTE_FLAG;
		set[1] = BYTE_A;
		set[2] = BYTE_C_SET;
		set[3] = BYTE_A ^ BYTE_C_SET;
		set[4] = BYTE_FLAG;

		while(flag && counter < ll.numTransmissions) {    
			if( write_serial(fd, set, 5) == -1 ) return -1;
		    alarm(ll.timeOut);
		    flag = 0;
		    if( read_serial(fd, buffer) == 0 ) {
				int a;
				for(a = 0; a < 5; a++)
					printf("%x - received\n", buffer[a]);

				//verificar conteudo
				printf("Recebeu mensagem, kappa\n");
				break;
			} 
		}
	} else {	

		if( read_serial(fd, buffer) == 0 ) {
			
			int a;
			for(a = 0; a < 5; a++)
				printf("%x - received\n", buffer[a]);

			unsigned char ua[5];
			ua[0] = BYTE_FLAG;
			ua[1] = BYTE_A;
			ua[2] = BYTE_C_UA;
			ua[3] = BYTE_A ^ BYTE_C_UA;
			ua[4] = BYTE_FLAG;
			write_serial(fd, ua, 5);
			printf("Recebeu mensagem, kappa fabullous!\n");
		}
	}

	/* Dependendo do status vai-se enviar (TRANSMITTER) ou receber (RECEIVER) uma mensagem */
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

int open_serial(int porta) {
    int fd;
    struct termios newtio;

    if (porta != 0 && porta != 1) {
		printf("Numero errado da porta\n");
        return -1;
    }

    ll.baudrate = BAUDRATE;
    if (sprintf(ll.port, "/dev/ttyS%d", porta) < 0) {
		printf("Erro ao criar string da porta\n");
        return -1;
    }

    ll.sequenceNumber = 0;
    ll.timeOut = 3;
    ll.numTransmissions = 3;
    
    fd = open(ll.port, O_RDWR | O_NOCTTY);
    if (fd < 0) {
		printf("Erro ao abrir fd\n");
        return -1;
    }

    if (tcgetattr(fd, &oldtio) == -1) {
		printf("Erro ao aceder aos atributos\n");
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
		printf("Erro ao alterar os atributos\n");
        return -1;
    }

    return fd;
}

int close_serial(int fd) {
    if (tcsetattr(fd, TCSANOW, &oldtio) == -1) {
        return -1;
    }
    close(fd);
    return 0;
}

int write_serial(int fd, unsigned char msg[], int length) {
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

int read_serial(int fd, unsigned char *buf) {
    /* Ler UA */
    int hasFirst = 0, nfr = 0;
    int iter = 0;
    while(1) {
        int n = 0;
        do {
            n = read(fd, buf, MAX_LEN - nfr);
        } while( n <= 0 && flag == 0 );

        if( n <= 0 )
            return -1;

        nfr += n;

        if( !hasFirst ) {
            int k;

            for( k = 0; k < nfr; k++ ) 
                if( buf[k] == BYTE_FLAG )
                    break;

            if( k < nfr ) {
                if( k ) {
                    memmove(buf, buf + k, nfr - k);
                    nfr -= k;
                }
            
                hasFirst = 1;
            }
            else
                nfr = 0;
        }
        
        if(hasFirst && nfr > 1) {
            int k;

            for( k = 1; k < nfr; k++ )
                if( buf[k] == BYTE_FLAG )
                    break;

            if( k < nfr ) {
                alarm(0);
                break;
                /* Para a tua mãe
				if(nfr > k+1)
                    memmove(buf, buf + k + 1, nfr - k + 1);
                nfr -= (k+1);
                hasFirst = 0;*/
            }
        }

        if( nfr == MAX_LEN )
            nfr = 0;

        iter++;
    }

    int a;
    for(a = 0; a < 5; a++)
        printf("%x - received\n", buf[a]);
	return 0;
}
