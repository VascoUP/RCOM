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
/*
TO DO:
    - Usar build_frame_us em todas as situacoes possiveis quando estiver testado
    - Testar build_frame_i, depois de testada, substituir em todas as situacoes possiveis
    - Adicionar mais quando nos formos lembrando
*/

static linklayer ll;
static struct termios oldtio;
static int flag = 1;
static int counter = 0;

static int handleMessage(unsigned int length, unsigned char msg[], int type_a);

static int destuffing(unsigned char **buffer, unsigned int length);
static int stuffing(unsigned char **buffer, unsigned int length);

static int open_serial(int porta, int status, int baudrate, int timeOut, int numTransmisions);
static int close_serial(int fd);
static int write_serial(int fd, unsigned char *msg, unsigned int length);
static int read_serial(int fd, unsigned char *buf);

static unsigned char* build_frame_us(char address, int sequence_number, int type);
static int build_frame_i(char address, int sequence_number, unsigned char **data, unsigned int length);

void atende() {
    if( ++counter == ll.numTransmissions )
        printf("Disconneted\n");
    else
        printf("Resending\n");
    incTimeOut();
    counter++;
    flag = 1;
}

int handleMessage(unsigned int length, unsigned char msg[], int type_a) {
int i, type = UNDEFINED;
unsigned char dataBcc = 0;

unsigned char f1 = 0, a = 0, c = 0, bcc1 = 0, bcc2 = 0;
    for( i = 0; i < length; i++ ) {
        //Flag - 1
        if( f1 == 0 ) {
            if( msg[i] == BYTE_FLAG ) {
                f1 = msg[i];
            }
        //Campo de endereco (tem de vir logo a seguir à primeira Flag)
        } else if( a == 0 && i > 0 && msg[i-1] == f1 ) {
            if( (msg[i] == BYTE_AT && type_a == A_T) || (msg[i] == BYTE_AR && type_a == A_R) ) {
              a = msg[i];
            }
            else {//Se nao for o campo de endereco returnar erro
              return ERR;
            }
        //Campo de controlo (tem de vir logo a seguir ao campo de endereco)
        } else if( msg[i-1] == a && type == UNDEFINED ) {
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
            case BYTE_C_RR2:
                type = TRAMA_RR;
                break;
            case BYTE_C_REJ:
            case BYTE_C_REJ2:
                type = TRAMA_REJ;
                break;
            default: //Se nenhum deles corresponder a um campo de controlo valido
                return ERR;
            }
              c = msg[i];
        //Campo de protecao (tem de vir antes do campo de controlo)
        } else if( msg[i-1] == c && bcc1 == 0 ) {
            if( (a ^ c) == msg[i] ) {
                bcc1 = msg[i];
            }
            else {
                return ERR;
              }
        //Flag - 2 (tem de vir antes do campo de protecao
        //              MENOS quando se trata de uma trama I)
        } else if( msg[i] == BYTE_FLAG && type == TRAMA_I ) {
            if( msg[i-1] == dataBcc ) {
                return type;
            }
        } else if( msg[i] == BYTE_FLAG && msg[i-1] == bcc1 && bcc2 == 0 ) {
            if( type != TRAMA_I )
                return type;
            else {
                return ERR;
            }
        } else if(bcc1 != 0) {
            if( dataBcc == 0 ) {
                dataBcc = msg[i];
            }
            else if( i + 1 != length && msg[i+1] != BYTE_FLAG) {
                dataBcc ^= msg[i];
            }
        }
    }
    return ERR;
}

int destuffing(unsigned char **buffer, unsigned int length) {
    int i = 0, count = 0;

    for (i = 0; i < length; i++) {
        if (*(*buffer + i) == BYTE_ESCAPE) {
            memmove(*buffer + i, *buffer + i + 1, length - i);
            *(*buffer + i) = *(*buffer + i) ^ 0x20;
            printf("0x%02x\n", *(*buffer + i));
            count++;
        }
    }
    int newlength = length - count;
    printf("%d %d %d\n", length, count, newlength);
	  if( newlength <= 0 ) {
		    printf("destuffing:: New array length is invalid\n");
		      return -1;
	  }

    unsigned char *temp = realloc(*buffer, newlength * sizeof(unsigned char));
    if (temp == NULL) {
        return -1;
    }
    *buffer = temp;

    return newlength;
}

int stuffing(unsigned char **buffer, unsigned int length) {
    int i = 0, count = 0;
    for (i = 0; i < length; i++) {
        if (*(*buffer + i) == BYTE_FLAG || *(*buffer + i) == BYTE_ESCAPE) {
            count++;
        }
    }

    int newlength = length + count;

    unsigned char *temp = realloc(*buffer, newlength * sizeof(unsigned char));
    if (temp == NULL) {
        return -1;
    }
    *buffer = temp;

    i = 0;
    for (i = 0; i < newlength; i++) {
        if (*(*buffer + i) == BYTE_FLAG || *(*buffer + i) == BYTE_ESCAPE) {
            memmove(*buffer + i + 1, *buffer + i, newlength - i);
            *(*buffer + i) = BYTE_ESCAPE;
            *(*buffer + i + 1) = *(*buffer + i + 1) ^ 0x20;
        }
    }

    return newlength;
}

int llopen(int porta, int status, int baudrate, int timeOut, int numTransmissions) {
    int fd;
    if((fd = open_serial(porta, status, baudrate, timeOut, numTransmissions)) == -1)  {
        printf("Erro open_serial\n");
        return -1; //Retornar logo se der erro
    }

    setStatistics();

    struct sigaction actionAlarm;
    actionAlarm.sa_handler = atende;
    sigemptyset(&actionAlarm.sa_mask);
    actionAlarm.sa_flags = 0;

    if(sigaction(SIGALRM, &actionAlarm, NULL) < 0){
        fprintf(stderr, "Error activating the alarm\n");
        return -1;
    }

    unsigned char buffer[MAX_LEN];
    int k;

    if( ll.status == TRANSMITTER ) {
        unsigned char *set = build_frame_us(BYTE_AT, ll.sequenceNumber, TRAMA_SET);

        while(flag && counter < ll.numTransmissions) {
            if( write_serial(fd, set, FRAMA_US_LEN) == -1 ) return -1;
            alarm(ll.timeOut);
            flag = 0;
            if( ( k = read_serial(fd, buffer) ) != -1 ) {
                if( handleMessage(k, buffer, A_T) == TRAMA_UA ) {
                    break;
                } else {
                    alarm(0);
                    counter++;
                    flag = 1;
                }
            }
        }
        if (counter == ll.numTransmissions) {
            printf("Maximum number of transmissions\n");
            return -1;
        }

        flag = 1;
        counter = 0;

	      free(set);
    } else {
        do {
            k = read_serial(fd, buffer);
            if( k == -1 ) {
                printf("llopen:: Error reading from the serial port\n");
                return -1;
            }
        } while( handleMessage(k, buffer, A_T) != TRAMA_SET );

        unsigned char *ua = build_frame_us(BYTE_AT, ll.sequenceNumber, TRAMA_UA);

        write_serial(fd, ua, FRAMA_US_LEN);

	      free(ua);
    }

    return fd;
}

int llclose(int fd) {
    flag = 1;
    counter = 0;

    int k;

    unsigned char *ua = build_frame_us(BYTE_AR, ll.sequenceNumber, TRAMA_UA);
    unsigned char *disc = build_frame_us(BYTE_AT, ll.sequenceNumber, TRAMA_DISC);

    unsigned char buffer[MAX_LEN];

    if( ll.status == TRANSMITTER ) {

        while(flag && counter < ll.numTransmissions) {
            if( write_serial(fd, disc, FRAMA_US_LEN) == -1 ) return -1;

            alarm(ll.timeOut);
            flag = 0;
            if( ( k = read_serial(fd, buffer) ) != -1 ) {
                if( handleMessage(k, buffer, A_T) == TRAMA_DISC ) {
                    flag = 1;
                    counter = 0;

                    break;
                } else {
                    alarm(0);
                    counter++;
                    flag = 1;
                }
            }
        }

        do {
          if( write_serial(fd, ua, FRAMA_US_LEN) == 0 ) {
            break;
          }
          counter++;
       } while(counter < ll.numTransmissions && k > 0);

    } else {
        do {
            k = read_serial(fd, buffer);
            if( k == -1 )
                return -1;
        } while( handleMessage(k, buffer, A_T) != TRAMA_DISC );

        write_serial(fd, disc, FRAMA_US_LEN);

        do {
            k = read_serial(fd, buffer);
            if( k == -1 )
                return -1;
        } while( handleMessage(k, buffer, A_R) != TRAMA_UA );
    }

    free(ua);
    free(disc);

    return close_serial(fd);
}

int llread(int fd, unsigned char ** buffer) {
    /*
      1 - Espera leitura de trama I
      2 - Enviar trama RR se leu mensagem com sucesso
          (Reijeitar caso contrário, trama REJ)
      3 - Returnar o que leu, ou negativo se deu erro
    */

    flag = 1;
    counter = 0;

    int n = -1;
    unsigned char* msg =  malloc(MAX_LEN * sizeof(char));
    if( (n = read_serial(fd, msg)) == -1)
      return -1;

    if ( handleMessage(n, msg, A_T) == TRAMA_I ) {
        unsigned char *rr = build_frame_us( BYTE_AT, ll.sequenceNumber, TRAMA_RR);
        write_serial(fd, rr, FRAMA_US_LEN);

        if( //Se sequenceNumber == 0 entao o BIT(6) == 1
            (msg[2] & BIT(6) && ll.sequenceNumber == 1) ||
            //Se sequenceNumber == 1 entao o BIT(6) == 0
            (!(msg[2] & BIT(6)) && ll.sequenceNumber == 0)) {
            //Duplicado
	    incFrameRepeat();
            printf("llread:: Duplicated\n");
        } else
            ll.sequenceNumber = ll.sequenceNumber == 0 ? 1 : 0;

        n -= 6;
        memmove(msg, msg + 4 * sizeof(unsigned char), n);

        if( destuffing(&msg, n) != -1 ) {
          *buffer = msg;

	incFrameReceive();

          return n; //return # characters read | -1 if error
        }

    }

    printf("llread:: Rejected packet\n");
    unsigned char *rej = build_frame_us( BYTE_AT, ll.sequenceNumber, TRAMA_REJ);
    incREJReceive();
    write_serial(fd, rej, FRAMA_US_LEN);
    return -1;

}

int llwrite(int fd, unsigned char *buffer, unsigned int length) {
    /*
        1 - Enviar trama de informacao com <length> bytes mais os bytes de controlo
            -> Poder acontecer não conseguir enviar, das duas uma:
                1. Ou espera que de timeout (alarm)
                2. Ou avanca para a proxima iteracao e incrementar o counter
        2 - Esperar leitura de RR (pode ser REJ)
            -> Se ler RR acaba a funcao
            -> Da timeout o que quer dizer que tem de reenviar a trama I exatamente igual
            -> Se receber REJ voltar a enviar, com isto tem de se incrementar o counter
        3 - Dado sucesso de envio (acaba por receber RR) returnar 0, caso contrário, returnar negativo
    */

    flag = 1;
    counter = 0;

    unsigned char resp[MAX_LEN];
    int k, tr, n = stuffing(&buffer, length);
    if( n < 0 ) {
        printf("llwrite:: Error stuffing the packet\n");
        return n;
    }

    n = build_frame_i(BYTE_AT, ll.sequenceNumber, &buffer, n);

    while(flag && counter < ll.numTransmissions) {
        if( write_serial(fd, buffer, n) == -1 ) return -1;

        do {
            alarm(ll.timeOut);
            flag = 0;
            if( ( k = read_serial(fd, resp) ) != -1 )
                tr = handleMessage(k, resp, A_T);
        } while( flag == 0 && tr != TRAMA_RR && tr != TRAMA_REJ );

        if( tr == TRAMA_RR ) {
            ll.sequenceNumber = ll.sequenceNumber == 0 ? 1 : 0;
            flag = 1;
            counter = 0;

            break;
        } else if( tr == TRAMA_REJ ) {
            printf("llwrite:: Packet rejected\n");
	        incREJSend();
            alarm(0);
            counter++;
            flag = 1;
        }
    }

    incFrameSend();

    return counter == ll.numTransmissions ? -1 : n; //return # characters written | -1 if error
}

int open_serial(int porta, int status, int baudrate, int timeOut, int numTransmissions) {
    int fd;
    struct termios newtio;

    if (porta != 0 && porta != 1) {
        printf("Numero errado da porta\n");
        return -1;
    }

    ll.baudrate = baudrate;
    if (sprintf(ll.port, "/dev/ttyS%d", porta) < 0) {
        printf("Erro ao criar string da porta\n");
        return -1;
    }

    ll.status = status;
    ll.sequenceNumber = status == TRANSMITTER ? 0 : 1;
    ll.timeOut = timeOut;
    ll.numTransmissions = numTransmissions;

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

int write_serial(int fd, unsigned char *msg, unsigned int length) {
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
    if( buf == NULL )
      return -1;

    unsigned int max_len = MAX_LEN;
    int hasFirst = 0, nfr = 0;
    int iter = 0;
    int k;
    while(1) {
        int n = 0;
        n = read(fd, buf + nfr, max_len - nfr);

        if( n <= 0 )
            return -1;

        nfr += n;

        if( !hasFirst ) {
            //int k;
            for( k = 0; k < nfr; k++ ) {
                if( buf[k] == BYTE_FLAG ) {
                    break;
                }
			}

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
            for( k = 1; k < nfr; k++ ) {
                if( buf[k] == BYTE_FLAG )
                    break;
            }

            if( k < nfr ) {
				if( handleMessage(k+1, buf, A_T) != ERR || handleMessage(k+1, buf, A_R) != ERR ) {
				    alarm(0);
				    break;
				} else {
  					memmove(buf, buf + k, nfr - k);
  					nfr -= k;
				}
            }
        }

        if( nfr == max_len )
            nfr = 0;

        iter++;
    }

    return k+1;
}

int build_frame_i(char address, int sequence_number, unsigned char **data, unsigned int length) {
    unsigned int frame_length = length + 6;
    unsigned char bcc2 = *(*data);

    int i;
    for(i = 1; i < length; i++)
        bcc2 ^= (*data)[i];

    unsigned char *tmp = realloc(*data, sizeof(unsigned char) * frame_length);
    if (tmp == NULL)
        return -1;

    *data = tmp;
    memmove(*data + 4, *data, length);

    *(*data) = BYTE_FLAG;
    *(*data + 1) = address;
    *(*data + 2) = BYTE_C_I | ((sequence_number) ? BIT(6) : 0);
    *(*data + 3) = *(*data + 1) ^ *(*data + 2);

    *(*data + frame_length - 2) = bcc2;
    *(*data + frame_length - 1) = BYTE_FLAG;

    return frame_length;
}

unsigned char* build_frame_us(char address, int sequence_number, int type) {
    unsigned char *frame = malloc(sizeof(char) * FRAMA_US_LEN);
    if (frame == NULL)
        return NULL;

    frame[0] = BYTE_FLAG;
    frame[1] = address;

    switch (type) {
        case TRAMA_SET:
            frame[2] = BYTE_C_SET;
            break;
        case TRAMA_DISC:
            frame[2] = BYTE_C_DISC;
            break;
        case TRAMA_UA:
            frame[2] = BYTE_C_UA;
            break;
        case TRAMA_RR:
            frame[2] = BYTE_C_RR | ((sequence_number) ? BIT(7) : 0);
            break;
        case TRAMA_REJ:
            frame[2] = BYTE_C_REJ | ((sequence_number) ? BIT(7) : 0);
            break;
        default:
            return NULL;
    }

    frame[3] = frame[1] ^ frame[2];
    frame[4] = BYTE_FLAG;

    return frame;
}

void free_frame(unsigned char *frame) {
    free(frame);
}
