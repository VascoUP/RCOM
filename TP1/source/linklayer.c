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

static int handleMessage(unsigned int length, unsigned char msg[], int type_a);

static int destuffing(unsigned char *buffer, unsigned int length);
static int stuffing(unsigned char *buffer, unsigned int length);

static int open_serial(int porta, int status);
static int close_serial(int fd);
static int write_serial(int fd, unsigned char msg[], unsigned int length);
static int read_serial(int fd, unsigned char *buf);

void atende() {
    printf("alarme #%d\n", ++counter);
    flag = 1;
}

// !!NOT FINISHED!!
int handleMessage(unsigned int length, unsigned char msg[], int type_a) {
    int i, type = UNDEFINED;
    unsigned char f1 = 0, a = 0, c = 0, bcc1 = 0, bcc2 = 0;
    for( i = 0; i < length; i++ ) {
        //Flag - 1
        if( f1 == 0 ) {
            if( msg[i] == BYTE_FLAG ) {
                printf("0 FLAG: 0x%x\n", msg[i]);
                f1 = msg[i];
            }
        //Campo de endereco (tem de vir logo a seguir à primeira Flag)
        } else if( a == 0 && i > 0 && msg[i-1] == f1 ) {
            if( (msg[i] == BYTE_AT && type_a == A_T) || (msg[i] == BYTE_AR && type_a == A_R) ) {
                printf("1 FLAG: 0x%x\n", msg[i]);
                a = msg[i];
            }
            else //Se nao for o campo de endereco returnar erro
                return ERR;
        //Campo de controlo (tem de vir logo a seguir ao campo de endereco)
        } else if( msg[i-1] == a && type == UNDEFINED ) {
            switch( msg[i] ) {
                case BYTE_C_I:
                case BYTE_C_I2:
                    type = TRAMA_I;
                    printf("Trama I\n");
                    break;
                case BYTE_C_SET:
                    type = TRAMA_SET;
                    printf("Trama SET\n");
                    break;
                case BYTE_C_DISC:
                    type = TRAMA_DISC;
                    printf("Trama DISC\n");
                    break;
                case BYTE_C_UA:
                    type = TRAMA_UA;
                    printf("Trama UA\n");
                    break;
                case BYTE_C_RR:
                case BYTE_C_RR2:
                    type = TRAMA_RR;
                    printf("Trama RR\n");
                    break;
                case BYTE_C_REJ:
                case BYTE_C_REJ2:
                    type = TRAMA_REJ;
                    printf("Trama REJ\n");
                    break;
                default: //Se nenhum deles corresponder a um campo de controlo valido
                    return ERR;
            }
            printf("2 FLAG: 0x%x\n", msg[i]);
            c = msg[i];
            printf("0x%x - 0x%x\n", a, c);
        //Campo de protecao (tem de vir antes do campo de controlo)
        } else if( msg[i-1] == c ) {
            if( (a ^ c) == msg[i] ) {
                printf("3 FLAG: 0x%x\n", msg[i]);
                bcc1 = msg[i];
            }
            else
                return ERR;
        //Flag - 2 (tem de vir antes do campo de protecao
        //              MENOS quando se trata de uma trama I)
        } else if( msg[i] == BYTE_FLAG && msg[i-1] == bcc1 && bcc2 == 0 ) {
            printf("4 FLAG: 0x%x\n", msg[i]);
            if( type != TRAMA_I )
                return type;
            else
                return ERR;
        //Segundo campo de protecao (so valido para tramas I)
        } else if( bcc1 != 0 && msg[i] == bcc1 && msg[i-1] != bcc1 && type == TRAMA_I ) {
            printf("5 FLAG: %x\n", msg[i]);
            bcc2 = msg[i];
        //Flag - 2 (so valido
        } else if( bcc2 != 0 && msg[i] == BYTE_FLAG && msg[i-1] == bcc2 && type == TRAMA_I ) {
            printf("%d\n", type);
            return type;
        }
    }

    return -1;
}

int destuffing(unsigned char *buffer, unsigned int length) {
    return 0;
}

int stuffing(unsigned char *buffer, unsigned int length) {
    int i = 0, count = 0;
    for (i = 0; i < length; i++) {
        if (buffer[i] == BYTE_FLAG || buffer[i] == BYTE_ESCAPE) {
            count++;
        }
    }

    int newlength = length + count;
    unsigned char *temp = realloc(buffer, newlength);
    if (temp == NULL) {
        return -1;
    }
    buffer = temp;

    i = 0;
    for (i = 0; i < newlength; i++) {
        if (buffer[i] == BYTE_FLAG || buffer[i] == BYTE_ESCAPE) {
            memmove(&buffer[i + 1], &buffer[i], newlength - i + 1);
            buffer[i] = BYTE_ESCAPE;
            buffer[i+1] = buffer[i+1] ^ 0x20;
        }
    }

    return newlength;
}

int llopen(int porta, int status) {
    int fd;
    if((fd = open_serial(porta, status)) == -1)  {
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
/*
    unsigned char buffer[MAX_LEN];
    int k;

    if( ll.status == TRANSMITTER ) {
        unsigned char set[5];
        set[0] = BYTE_FLAG;
        set[1] = BYTE_AT;
        set[2] = BYTE_C_SET;
        set[3] = set[1] ^ set[2];
        set[4] = BYTE_FLAG;

        while(flag && counter < ll.numTransmissions) {
            if( write_serial(fd, set, 5) == -1 ) return -1;
            alarm(ll.timeOut);
            flag = 0;
            if( ( k = read_serial(fd, buffer) ) != -1 ) {
                if( handleMessage(k, buffer, A_T) == TRAMA_UA ) {
                    printf("Recebeu mensagem, kappa\n");
                    break;
                } else {
                    counter++;
                    flag = 1;
                }
            }
        }
        if (counter == ll.numTransmissions)
            printf("Maximum number of transmissions\n");

        flag = 1;
        counter = 0;
    } else {
        do {
            k = read_serial(fd, buffer);
            if( k == -1 )
                return -1;
        } while( handleMessage(k, buffer, A_T) != TRAMA_SET );

        unsigned char ua[5];
        ua[0] = BYTE_FLAG;
        ua[1] = BYTE_AT;
        ua[2] = BYTE_C_UA;
        ua[3] = ua[1] ^ ua[2];
        ua[4] = BYTE_FLAG;
        write_serial(fd, ua, 5);
        printf("Recebeu mensagem, kappa fabullous!\n");
    }
*/
    return fd;
}

int llclose(int fd) {
/*
    int k;
    unsigned char buffer[MAX_LEN];
    unsigned char disc[5];
    unsigned char ua[5];

    disc[0] = BYTE_FLAG;
    disc[1] = BYTE_AT;
    disc[2] = BYTE_C_DISC;
    disc[3] = disc[1] ^ disc[2];
    disc[4] = BYTE_FLAG;

    ua[0] = BYTE_FLAG;
    ua[1] = BYTE_AR;    //Emissor will be sending UA as a response
    ua[2] = BYTE_C_UA;
    ua[3] = ua[1] ^ ua[2];
    ua[4] = BYTE_FLAG;

    if( ll.status == TRANSMITTER ) {

        while(flag && counter < ll.numTransmissions) {
            if( write_serial(fd, disc, 5) == -1 ) return -1;
            alarm(ll.timeOut);
            flag = 0;
            if( ( k = read_serial(fd, buffer) ) != -1 ) {
                if( handleMessage(k, buffer, A_T) == TRAMA_DISC ) {
                    printf("Recebeu mensagem, kappa disc\n");

                    flag = 1;
                    counter = 0;

                    break;
                } else {
                    counter++;
                    flag = 1;
                }
            }
        }

        do {
            k = write_serial(fd, ua, 5); //return -1;
            counter++;
        } while(counter < ll.numTransmissions && k > 0);

    } else {
        do {
            k = read_serial(fd, buffer);
            if( k == -1 )
                return -1;
        } while( handleMessage(k, buffer, A_T) != TRAMA_DISC );

        write_serial(fd, disc, 5);

        do {
            k = read_serial(fd, buffer);
            if( k == -1 )
                return -1;
        } while( handleMessage(k, buffer, A_R) != TRAMA_UA );

        printf("Recebeu mensagem, kappa fabullous disc!\n");
    }
*/
    return close_serial(fd);

}

int llread(int fd, unsigned char * buffer) {
    /*
      1 - Espera leitura de trama I
      2 - Enviar trama RR se leu mensagem com sucesso
          (Reijeitar caso contrário, trama REJ)
      3 - Returnar o que leu, ou negativo se deu erro
    */
    int n = -1;
    unsigned char msg[255];
    n = read_serial(fd, msg);
    int a;
    for( a = 0; a < n; a++ )
        printf("%c\n", msg[a]);

    if ( handleMessage(n, msg, A_T) == TRAMA_I ) {
        if( //Se sequenceNumber == 0 entao o BIT(6) == 1
            (msg[2] & BIT(6) && ll.sequenceNumber == 0) ||
            //Se sequenceNumber == 1 entao o BIT(6) == 0
            (!(msg[2] & BIT(6)) && ll.sequenceNumber == 1)) {
            //Se nao e duplicado
            printf("Read I\n");

            unsigned char rr[5];
            rr[0] = BYTE_FLAG;
            rr[1] = BYTE_AT;
            rr[2] = BYTE_C_RR;
            rr[3] = rr[1] ^ rr[2];
            rr[4] = BYTE_FLAG;

            write_serial(fd, rr, 5);
        } else
            //Handle duplicado
            printf("Duplicado\n");
    } else {
        printf("Read NOTI\n");

        unsigned char rej[5];
        rej[0] = BYTE_FLAG;
        rej[1] = BYTE_AT;
        rej[2] = BYTE_C_REJ;
        rej[3] = rej[1] ^ rej[2];
        rej[4] = BYTE_FLAG;

        if( ll.sequenceNumber == 1 ) {
            ll.sequenceNumber = 0;
            rej[2] |= BIT(7);
        } else
            ll.sequenceNumber = 1;

        write_serial(fd, rej, 5);
    }
/*
    n -= 6;
    memmove(buffer, buffer + 4 * sizeof(unsigned char), n);

    destuffing(buffer, n);

    int a;
    for( a = 0; a < n; a++ )
    	printf("%c", buffer[a]);
    printf("\nEnd message\n");
*/
    return n; //return # characters read | -1 if error
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
    unsigned char resp[MAX_LEN];
    int k, tr, n = stuffing(buffer, length);
    if( n < 0 )
        return n;

    n += 6;
    buffer = (unsigned char *) realloc (buffer, n);
    memmove(buffer + 4 * sizeof(unsigned char), buffer, n - 6);

    buffer[0] = BYTE_FLAG;
    buffer[1] = BYTE_AT;
    buffer[2] = BYTE_C_I;
    buffer[3] = buffer[1] ^ buffer[2];
    buffer[n-1] = buffer[3];
    buffer[n-2] = BYTE_FLAG;
    printf("print message\n");    int a;
    for( a = 0; a < n; a++ )
        printf("0x%x\n", buffer[a]);
    printf("End message\n");

    if( ll.sequenceNumber == 1 ) {
        ll.sequenceNumber = 0;
        buffer[2] |= BIT(7);
    } else
        ll.sequenceNumber = 1;


    while(flag && counter < ll.numTransmissions) {
        alarm(ll.timeOut);

        if( write_serial(fd, buffer, n) == -1 ) return -1;

        do {
            flag = 0;
            if( ( k = read_serial(fd, resp) ) != -1 )
                tr = handleMessage(k, resp, A_T);
            printf("End read\n");
        } while( tr != TRAMA_RR && tr != TRAMA_REJ );

        if( tr == TRAMA_RR ) {
            printf("Recebeu mensagem, kappa ua\n");

            flag = 1;
            counter = 0;

            break;
        } else if( tr == TRAMA_REJ ) {
            printf("REJ\n");
            counter++;
            flag = 1;
        }
    }

    return counter == ll.numTransmissions ? -1 : n; //return # characters written | -1 if error
}

int open_serial(int porta, int status) {
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

    ll.status = status;
    ll.sequenceNumber = status == TRANSMITTER ? 0 : 1;
    ll.timeOut = 3;
    ll.numTransmissions = 3;

    printf("Before Open\n");
    fd = open(ll.port, O_RDWR | O_NOCTTY);
    printf("Abriu\n");
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

int write_serial(int fd, unsigned char msg[], unsigned int length) {
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
    int hasFirst = 0, nfr = 0;
    int iter = 0;
    int k;
    while(1) {
        int n = 0;

        n = read(fd, buf, MAX_LEN - nfr);

        if( n <= 0 )
            return -1;

        nfr += n;

        if( !hasFirst ) {
            //int k;
            for( k = 0; k < nfr; k++ )
                if( buf[k] == BYTE_FLAG ) {
                    printf("First flag: %d\n", k);
                    break;
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

            for( k = 1; k < nfr; k++ )
                if( buf[k] == BYTE_FLAG ) {
                    printf("Last flag: %d\n", k);
                    break;
                }

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

    return k + 1;
}
