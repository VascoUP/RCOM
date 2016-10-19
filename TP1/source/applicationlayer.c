
#include "linklayer.h"
#include <string.h>

typedef struct {
    int fileDescriptor;
    int status;         //TRANSMITTER | RECEIVER
} applicationLayer;

/*
BYTE_C_I	0x00
BYTE_C_SET	0x03
BYTE_C_UA	0x07
BYTE_C_DISC	0x0B
BYTE_C_RR	0x05
BYTE_C_REJ	0x01
*/

int handleMessage(int length, char msg[]) {
	int i, type;
	char f1 = '\0', a = '\0', c = '\0', bcc1 = '\0', bcc2 = '\0';
	for( i = 0; i < length; i++ ) {
		if( f1 == '\0' ) {
			if( msg[i] == BYTE_FLAG )
				f1 = msg[i];
		} else if( a == '\0' && i > 0 && msg[i-1] == f1 ) {
			if( msg[i] == BYTE_A )
				a = msg[i];
			else
				return ERR;
		} else if( i > 0 && msg[i-1] == a ) {
			switch( msg[i] ) {
				case BYTE_C_I:
				case BYTE_C_I & BIT(6):
					type = TRAMA_I;
					break;
				case BYTE_C_SET:
					type = TRAMA_SET;
					break;
				case BYTE_C_DISC:
					type = TRAMA_DISC;
					break;
				case BYTE_C_UA:
					type = TRAMA_UA;
					break;
				case BYTE_C_RR:
				case BYTE_C_RR & BIT(7):
					type = TRAMA_RR;
					break;
				case BYTE_C_REJ:
				case BYTE_C_REJ & BIT(7):
					type = TRAMA_REJ;
					break;
				default:
					return ERR;
			}
			c = msg[i];
		}
	} else if( i > 0 && msg[i-1] == c ) {
		if( a ^ c == msg[i] )
			bcc1 = msg[i];
		else
			return ERR;
	} else if( i > 0 && msg[i-1] == bcc1 ) {
		if( msg[i] == F ) {
			if( type != TRAMA_I )
				break;
			else
				return ERR;
		} else if( msg[i] != F ) {
			if( type != TRAMA_I )
				return ERR;
			/*
			else
				func_handler_info();
			*/
		}
	} else if( bcc1 != '\0' && msg[i] == bcc1 && type == TRAMA_I ) {
		bcc2 = msg[i];
	} else if( i > 0 && msg[i-1] == bccs && type == TRAMA_I ) {
		return type;
	}

	return 0;
}

int main(int argc, char **argv) {

	applicationLayer info;
	int port;

	if( strcmp("receiver", argv[1])==0 ) { 
		if( argc != 3 )
			return -1;

		info.status = RECEIVER;

	}
	else if( strcmp("transmitter", argv[1])==0 ) { 
		if( argc != 4 )
			return -1;

		info.status = TRANSMITTER;

	}
	else
		return -1;

	if( strcmp("/dev/ttyS0", argv[2])==0 )
		port = 0;
	else if( strcmp("/dev/ttyS1", argv[2])==0 )
		port = 1;
	else
		return -1;

	info.fileDescriptor = llopen( port, info.status );
	if( info.fileDescriptor < 0 )
		return -1;

	int count = 0;
	do {
		if( llclose(info.fileDescriptor) == 0 )
			break;
		count++;
	} while( count < 3 );

	if( count == 3 )
		return -1;
	return 0;
}
