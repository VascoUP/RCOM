/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <signal.h>


#define BAUDRATE B9600
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define MAX_LEN 255
#define MESSAGE_LENGTH  5


#define F           0x7E

#define A           0x03
#define C_SET       0x03
#define C_UA        0x07

#define BCC_SET     A ^ C_SET
#define BCC_UA      A ^ C_UA

volatile int STOP=FALSE;
int flag = 1, counter = 0;

void write_serial(int fd, char set[], int length) {
    int nw = 0;
    nw = write(fd,set,length);
    while(length > nw) {
        int n;
        n = write(fd, set+nw, length-nw);
        if(!n)
            break;
        nw += n;
    }
}

void read_serial(int fd) {
    /* Ler UA */
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

void atende() {
    printf("alarme #%d\n", ++counter);
    flag = 1;
}

int main(int argc, char** argv)
{
    int fd;
    struct termios oldtio, newtio;
    
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


    fd = open("/dev/ttyS0", O_RDWR | O_NOCTTY | O_NONBLOCK );
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
    newtio.c_cc[VMIN]     = 1;   /* blocking read until 5 chars received */

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

    (void) signal(SIGALRM, atende); 

    /*
        Escrever o SET para a porta de serie
    */

    char set[5];
    set[0] = F;
    set[1] = A;
    set[2] = C_SET;
    set[3] = BCC_SET;
    set[4] = F;

    while(flag && counter < 3) {
        write_serial(fd, set, MESSAGE_LENGTH);
        alarm(3);
        flag = 0;
        read_serial(fd);
    }

    if(counter == 3)
        perror("Couldn't send message");

    /* 
        O ciclo FOR e as instruções seguintes devem ser alterados de modo a respeitar 
        o indicado no guião 
    */

    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
            perror("tcsetattr");
        exit(-1);
    }

    close(fd);
    return 0;
}
