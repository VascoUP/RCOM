#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>

#include "statistics.h"
#include "interface.h"

#define BIT(n)      0x01 << n

#define UNDEFINED   -2

/* Macros to identify a byte of the message */
#define BYTE_FLAG   0x7E
#define BYTE_ESCAPE 0x7D
#define BYTE_AT     0x03
#define BYTE_AR     0x01
#define BYTE_C_I    0x00
#define BYTE_C_I2   0x40
#define BYTE_C_SET  0x03
#define BYTE_C_UA   0x07
#define BYTE_C_DISC 0x0B
#define BYTE_C_RR   0x05
#define BYTE_C_RR2  0x85
#define BYTE_C_REJ  0x01
#define BYTE_C_REJ2 0x81

/* Macros used to identify the type of message */
#define ERR         -1
#define TRAMA_I     0
#define TRAMA_SET   1
#define TRAMA_DISC  2
#define TRAMA_UA    3
#define TRAMA_RR    4
#define TRAMA_REJ   5

/* Macros used to identify the type of packet */
#define DATA_PACKET     1
#define START_PACKET    2
#define END_PACKET      3

#define FRAMA_US_LEN    5

#define A_T         0       //A Transmitter
#define A_R         1       //A Receiver

#define MAX_LEN     255     //buffer's maximum size
#define BAUDRATE    B9600

#define TRANSMITTER 0       //flag that indicate that this is the transmitter
#define RECEIVER    1       //flag that indicate that this is the receiver

typedef struct {
    int status;                     //TRANSMITTER || RECEIVER
    char port[20];                  //indentifier for the serial port
    int baudrate;                   //transmition rate
    unsigned int sequenceNumber;    //value = 0 | 1 ( alternate bettween each other )
    unsigned int timeOut;           //wait time before resending package
    unsigned int numTransmissions;  //maximum number of repeated transmissions
    unsigned int maxLengthTrama;
} linklayer;

int llopen(transmitterInfo info, int status);

int llclose(int fd);

int llread(int fd, unsigned char ** buffer);

int llwrite(int fd, unsigned char * buffer, unsigned int length);
