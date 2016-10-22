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

static int open_serial(int porta); 
static int close_serial(int fd);
static int write_serial(int fd, char msg[], int length);

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

    ll.baudrate = BAUDRATE;
    if (sprintf(ll.port, "/dev/ttyS%d", porta) < 0) {
        return -1;
    }
    
    fd = open(ll.port, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) {
        return -1;
    }

    if (tcgetattr(fd, &oldtio) == -1) {
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
        return -1;
    }

    return fd;
}

static int close_serial(int fd) {
    if (tcsetattr(fd, TCSANOW, &oldtio) == -1) {
        return -1;
    }
    close(fd);
    return 0;
}

static int write_serial(int fd, char msg[], int length) {
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
