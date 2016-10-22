#include "linklayer.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

static linklayer ll;
static struct termios oldtio;

static int open_serial(int porta); // return fd or -1 on error

int llopen(int porta, int status) {

    int fd = open_serial(porta);
    if (fd == -1) {
        return -1;
    }
    
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

static int open_serial(int porta) {
    int fd;
    struct termios newtio;

    if (porta != 0 || porta != 1) {
        return -1;
    }

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

    newtio.c_lflag = 0;

    newtio.c_cc[VTIME] = 0;
    newtio.c_cc[VMIN] = 1;

    tcflush(fd, TCIOFLUSH);

    if (tcsetattr(fd, TCSANOW, &newtio) == -1) {
        return -1;
    }

    return fd;
}
