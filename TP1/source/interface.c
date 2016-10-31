#include "interface.h"

receiverInfo receiver;
transmitterInfo transmitter;

void getInformation() {

	char* file = (char*) malloc(MAX_LENGTH * sizeof(char));
	printf("\nWrite the file's name: ");
	fgets(file, MAX_LENGTH, stdin); //diz que gets é perigoso

	strcpy(receiver.fileName, file);
	strcpy(transmitter.fileName, file);

	printf("%s - %s\n", receiver.fileName, transmitter.fileName);

	int maxLength;
	printf("\nChoose the maximum length of I frame: ");
	scanf("%d", &maxLength);
	fflush(stdin);

	while(maxLength < 0){
		printf("\nYou need to choose a positive number: ");
		scanf("%d", &maxLength);
		fflush(stdin);
	}
	
	int timeout;
	printf("\nChoose the time-out that you prefer in seconds (0 is the default value - 3 seconds): ");
	scanf("%d", &timeout);
	fflush(stdin);

	while(timeout < 0){
		printf("\nYou need to choose a positive number: ");
		scanf("%d", &timeout);
		fflush(stdin);
	}

	if(timeout == 0)
		timeout = DEFAULT_TIMEOUT;

	receiver.timeout = timeout;
	transmitter.timeout = timeout;

	printf("%d - %d\n", receiver.timeout, transmitter.timeout);

	int numTransmissions;
	printf("\nChoose the maximum number of transmissions that yoy prefer (0 is the default value - 3 transmissions): ");

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

	printf("%d\n", transmitter.timeout);

	int baudrate;
	printf("\nChoose the baudrate that you prefer:\n");
  	printf("0 - 1200\n1 - 1800\n2 - 2400\n3 - 4800\n4 - 9600\n5 - 38400\n6 - 57600\n7 - 115200\n");
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

	receiver.baudrate = baudrate;
	transmitter.baudrate = baudrate;

	printf("%d -%d\n", receiver.baudrate, transmitter.baudrate);
}

receiverInfo getReceiverInfo() {
  return receiver;
}

transmitterInfo getTransmitterInfo() {
  return transmitter;
}
