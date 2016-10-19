
#define BIT(n)		0x01 << n

#define BYTE_FLAG	0x7E
#define BYTE_A		0x03
#define BYTE_C_SET	0x03
#define BYTE_C_UA	0x07
#define BYTE_C_DISC	0x0B
#define BYTE_C_RR	0x05
#define BYTE_C_REJ	0x01		

#define MAX_SIZE    255 	//buffer's maximum size
#define BAUDRATE	B9600 	//

#define TRANSMITTER 0   	//flag that indicate that this is the transmitter
#define RECEIVER    1   	//flag that indicate that this is the receiver

typedef struct {
    char port[20];                  //indentifier for the serial port
    int baudrate;                   //transmition rate
    unsigned int sequenceNumber;    //value = 0 | 1 ( alternate bettween each other )
    unsigned int timeOut;           //wait time before resending package
    unsigned int numTransmissions;  //maximum number of repeated transmissions
    char frame[MAX_SIZE];           //image to be sent
} linkLayer;

int llopen(int porta, int status);

int llclose(int fd);

int llread(int fd, char * buffer);

int llwrite(int fd, char * buffer, int length);
