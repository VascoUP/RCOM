#include "linklayer.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

struct termios oldtio;

int llopen(int porta, int status) {

	if (porta != 0 || porta != 1) {
		return -1;
	}

	linkLayer linklayer;
	int fd;
	struct termios newtio;

	linklayer.baudrate = BAUDRATE;

	if (sprintf(linklayer.port, "/dev/ttyS%d", porta) < 0) {
		return -1;
	}
	
	fd = open(linklayer.port, O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (fd < 0) {
		return -1;
	}

	if (tcgetattr(fd, &oldtio) == -1) {
		return -1;
	}
	
	bzero(&newtio, sizeof(newtio));
	newtio.c_cflag = linklayer.baudrate | CS8 | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;
	

    return 0; //return fileDescriptor | -1 if error
}

int llclose(int fd) {
    /*
    ...
    */
    return 0; //return 0 if success | -1 if error
}

int llread(int fd, char * buffer) {
    /*
    ..
    */
    return 0; //return # characters read | -1 if error
}

int llwrite(int fd, char * buffer, int length) {
    /*
    ...
    */
    return 0; //return # characters written | -1 if error
}
