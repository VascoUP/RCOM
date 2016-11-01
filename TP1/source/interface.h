#ifndef INTERFACE_H
#define INTERFACE_H

#include <string.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>

#define MAX_LENGTH 		256

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
  int port;
  char fileName[MAX_LENGTH];
  int maxLengthTrama;
  int numTransmissions;
  int timeout;
  int baudrate;
}transmitterInfo;

void getInformationTransmitter();

transmitterInfo getTransmitterInfo();

#endif

