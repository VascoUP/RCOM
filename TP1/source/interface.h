#ifndef INTERFACE_H
#define INTERFACE_H

#include <string.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>

#define MAX_LENGTH 		256           //File name maximum size

#define DEFAULT_TIMEOUT		  3       //Default time out
#define DEFAULT_NUMTRANS   	3       //Deafult maximum number of transmissions

//Baudrates
#define BAUDRATE_1200      	B1200
#define BAUDRATE_1800      	B1800
#define BAUDRATE_2400      	B2400
#define BAUDRATE_4800      	B4800
#define BAUDRATE_9600     	B9600
#define BAUDRATE_38400     	B38400
#define BAUDRATE_57600     	B57600
#define BAUDRATE_115200    	B115200

//Information which the transmitter will choose
typedef struct {
  int port;                     //Serial port
  char fileName[MAX_LENGTH];    //File name
  int maxLengthTrama;           //Maximum frame length
  int numTransmissions;         //Maximum number of transmissions
  int timeout;                  //Time out
  int baudrate;                 //Baudrate
}transmitterInfo;

//Gets the information from terminal
void getInformationTransmitter();

#endif