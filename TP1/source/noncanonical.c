/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>

#define BAUDRATE B9600
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define MAX_LEN	255


#define F			0x7E

#define A			0x03
#define C_SET		0x03
#define C_UA		0x07

#define BCC_SET		A ^ C_SET
#define BCC_UA		A ^ C_UA

/* States */
#define START_ST	0
#define FLAG_RCV	1
#define A_RCV		2
#define C_RCV		3
#define BCC_RCV		4
#define STOP_ST		5

int current_state = START_ST;

volatile int STOP=FALSE;

int process(unsigned char buf[], int length, int fd) {
	int a;
	for( a = 0; a < length; a++ ) {
		printf("Process - %x\n", buf[a]);
	}

	/* Enviar UA */
	unsigned char ua[5];
	ua[0] = F;
	ua[1] = A;
	ua[2] = C_UA;
	ua[3] = BCC_UA;	
	ua[4] = F;

	int nw = 0;
	nw = write(fd, ua+nw, 5);
	while(5 > nw) {
		int n;
		n = write(fd, ua+nw, 5-nw);
		if(!n)
			break;
		nw += n;
	}

	return 0;
}

void write_serial(int fd, char msg[], int length) {
	/* Write message to serial port */
	int nw = 0;
	nw = write(fd,msg,length);
	while(length > nw) {
		int n;
		n = write(fd, msg+nw, length-nw);
		if(!n)
			break;
		nw += n;
	}
}

void read_message(int fd) {
	/* Read message from serial port */
	unsigned char buf[MAX_LEN];
	int hasFirst = 0, nfr = 0;
	int iter = 0;
	while(!STOP) {
		int n = 0;
		do {
			n = read(fd, buf, MAX_LEN - nfr);
		} while( n <= 0 && flag == 0 );

		if( n <= 0 )
			return ;

		nfr += n;

		if( !hasFirst ) {
			int k;

			for( k = 0; k < nfr; k++ ) 
				if( buf[k] == F )
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
				if( buf[k] == F )
					break;

			if( k < nfr ) {
				alarm(0);
				break;
				/*if(nfr > k+1)
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
}

int send_message(int fd, char msg[], int length) {
	/*
		Write a message to the serial port
		and wait for the confirmation
	*/

	while(flag && counter < 3) {
		write_serial(fd, msg, length_LENGTH);
		alarm(3);
		flag = 0;
		read_serial(fd);
	}

	if(counter == 3)
		return -1;
	return 0;
}

int open_serial(struct linklayer) {

    int fd;    
	struct termios oldtio,newtio;

    if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }

	
	/*
		Open serial port device for reading and writing and not as controlling tty

		because we don't want to get killed if linenoise sends CTRL-C.
	*/
	  
    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd <0) {perror(argv[1]); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 5;   /* blocking read until 5 chars received */

	 /* 
		VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
		leitura do(s) próximo(s) caracter(es)

	 */

    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    printf("New termios structure set\n");

	return fd;
}

int main(int argc, char** argv)
{
    int fd;
    struct termios oldtio,newtio;

    if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }


  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */
  
    
    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd <0) {perror(argv[1]); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 5;   /* blocking read until 5 chars received */



  /* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) próximo(s) caracter(es)
  */

    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    printf("New termios structure set\n");

	/* Ler SET */

	unsigned char buf[MAX_LEN];
	int hasFirst = 0, nfr = 0;
	int iter = 0;
	while(!STOP) {
		int n;
		n = read(fd, buf, MAX_LEN - nfr);
		int a;

		for( a = 0; a < n; a++ ) {
			printf("%d - %x\n", iter, buf[a]);
		}

		if( n <= 0 ) {
			STOP = TRUE;
			break;
		}

		nfr += n;

		if( !hasFirst ) {
			int k;

			for( k = 0; k < nfr; k++ ) 
				if( buf[k] == F )
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
				if( buf[k] == F )
					break;

			if( k < nfr ) {
				process(buf, k+1, fd);
				STOP = TRUE;
/*
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

    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
    return 0;
}
