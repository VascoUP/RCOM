/*
include ...
*/

#define MAX_SIZE    255 //buffer's maximum size

#define TRANSMITTER 0   //flag that indicate that this is the transmitter
#define RECEIVER    1   //flag that indicate that this is the receiver

typedef applicationLayer {
    int fileDescriptor;
    int status;         //TRANSMITTER | RECEIVER
} struct;

typedef linklayer {
    char port[20];                  //indentifier for the serial port
    int baudrate;                   //transmition rate
    unsigned int sequenceNumber;    //value = 0 | 1 ( alternate bettween each other )
    unsigned int timeOut;           //wait time before resending package
    unsigned int numTransmissions;  //maximum number of repeated transmissions
    char frame[MAX_SIZE];           //image to be sent
} struct;

int llopen(int porta, int status) {
    /*
    ...
    */
    return 0; //return fileDescriptor | -1 if error
}

int llclose(int fd) {
    /*
    ...
    */
    return 0; //return 0 if success | -1 if error
}

int llread() {
    /*
    ..
    */
    return 0; //return # characters read | -1 if error
}

int llwrite() {
    /*
    ...
    */
    return 0; //return # characters written | -1 if error
}
