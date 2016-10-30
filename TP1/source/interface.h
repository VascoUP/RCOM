#include <string.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>

#define MAX_LENGTH 			256

#define DEFAULT_TIMEOUT		3
#define DEFAULT_NUMTRANS   	3

#define BAUDRATE_1200      	B1200
#define BAUDRATE_1800      	B1800
#define BAUDRATE_2400      	B2400
#define BAUDRATE_4800      	B4800
#define BAUDRATE_9600     	B9600
#define BAUDRATE_38400     	B38400
#define BAUDRATE_57600     	B57600
#define BAUDRATE_115200    	B115200

typedef struct {
  char fileName[MAX_LENGTH];
  int timeout;
  int baudrate; //baurate é int??????????
}receiverInfo;

typedef struct {
  char fileName[MAX_LENGTH];
  int numTransmissions;
  int timeout;
  int baudrate; //baurate é int??????????
}transmitterInfo;

void getInformation();

receiverInfo getReceiverInfo();

transmitterInfo getTransmitterInfo();

