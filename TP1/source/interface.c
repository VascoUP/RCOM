#include "interface.h"

transmitterInfo transmitter;

void getInformationTransmitter(){
	//Serial port infomation
	printf("\n-> Choose the serial port name:\n");
	printf("0 - /dev/ttyS0\n1 - /dev/ttyS1\n");
	printf("\nAnswer: ");

	int answer;
	scanf("%d", &answer);
	fflush(stdin);

	if(answer != 0 && answer!= 1){
		while( answer != 0 || answer != 1){
			printf("\nYou have to choose between 0 or 1\n\nChose the serial port name: ");
			scanf("%d", &answer);
			fflush(stdin);
		}
	}

	transmitter.port = answer;

	//File information
	char file[MAX_LENGTH];
	printf("\n-> Write the file's name: ");
	scanf("%s", file);
	fflush(stdin);

	strcpy(transmitter.fileName, file);

	int maxLength;
	printf("\n-> Choose the maximum length of I frame: ");
	scanf("%d", &maxLength);
	fflush(stdin);

	while(maxLength < 0){
		printf("\nYou need to choose a positive number: ");
		scanf("%d", &maxLength);
		fflush(stdin);
	}

	//File size
	int file_size;
	FILE *fp;
	fp = fopen(file, "r");
	fseek(fp, 0, SEEK_END);
	file_size = ftell(fp);
	rewind(fp);

	//If maximum length of I frame is bigger than file size
	if(file_size < maxLength){
		while(file_size < maxLength){
			printf("\nThe value needs to bo lower than %d, because %d is the file size...\n", file_size, file_size);
			printf("\n-> Choose the maximum length of I frame: ");
			scanf("%d", &maxLength);
			fflush(stdin);

			while(maxLength < 0){
			printf("\nYou need to choose a positive number: ");
			scanf("%d", &maxLength);
			fflush(stdin);
			}
		}
	}

	fclose(fp);
	transmitter.maxLengthTrama = maxLength;

	//Time out
	int timeout;
	printf("\n-> Choose the time-out that you prefer in seconds (0 is the default value - 3 seconds): ");
	scanf("%d", &timeout);
	fflush(stdin);

	while(timeout < 0){
		printf("\nYou need to choose a positive number: ");
		scanf("%d", &timeout);
		fflush(stdin);
	}

	if(timeout == 0)
		timeout = DEFAULT_TIMEOUT;

	transmitter.timeout = timeout;

	//Maximum number of transmissions
	int numTransmissions;
	printf("\n-> Choose the maximum number of transmissions that yoy prefer (0 is the default value - 3 transmissions): ");

	scanf("%d", &numTransmissions);
	fflush(stdin);

	while(numTransmissions < 0){
		printf("\nYou need to choose a positive number: ");
		scanf("%d", &numTransmissions);
		fflush(stdin);
	}

	if(numTransmissions == 0)
		numTransmissions = DEFAULT_NUMTRANS;

	transmitter.numTransmissions = numTransmissions;

	//Baudrate
	int baudrate;
	printf("\n-> Choose the baudrate that you prefer:\n");
  	printf("0 - B1200\n1 - B1800\n2 - B2400\n3 - B4800\n4 - B9600\n5 - B38400\n6 - B57600\n7 - B115200\n");
  	printf("\nInsert your choice: ");
  	scanf("%d", &baudrate);
 	fflush(stdin);

  	while (baudrate < 0 || baudrate > 7) {
    	printf("Insert a value between 0 and 7: ");
    	scanf("%d", &baudrate);
    	fflush(stdin);
  	}
  
	switch (baudrate) {
   		case 0:
      		baudrate = BAUDRATE_1200;
    		break;
    	case 1:
      		baudrate = BAUDRATE_1800;
    		break;
		case 2:
      		baudrate = BAUDRATE_2400;
    		break;
    	case 3:
      		baudrate = BAUDRATE_4800;
			break;
		case 4:
			baudrate = BAUDRATE_9600;
			break;
		case 5:
			baudrate = BAUDRATE_38400;
			break;
		case 6:
			baudrate = BAUDRATE_57600;
			break;
		case 7:
			baudrate = BAUDRATE_115200;
			break;
	}

	transmitter.baudrate = baudrate;
}

transmitterInfo getTransmitterInfo() {
  return transmitter;
}
