
#define MAX_SIZE    255 //buffer's maximum size
#define BAUDRATE	B9600 //

#define TRANSMITTER 0   //flag that indicate that this is the transmitter
#define RECEIVER    1   //flag that indicate that this is the receiver

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
