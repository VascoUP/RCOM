#include "FTPClient.h"

char* getHostIP(char* host){
	struct hostent *h = gethostbyname(host);
	if (h == NULL) {
		herror("gethostbyname");
		return NULL;
	}

	char *IP = inet_ntoa(*((struct in_addr *)(h->h_addr)));
	return IP;
}

int connectSocket(char* IP, int port){
	int socketfd;
	struct	sockaddr_in server_addr;

	/*server address handling*/
	bzero((char*)&server_addr,sizeof(server_addr)); //shall place sizeof(server_addr) zero-valued bytes in the area pointed to by server_addr.
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(IP);	/*32 bit Internet address network byte ordered*/
	server_addr.sin_port = htons(port);				/*server TCP port must be network byte ordered */

	/*open an TCP socket*/
	if ((socketfd = socket(AF_INET,SOCK_STREAM,0)) < 0)
	{
    	perror("socket() error");
        exit(1);
    }

	/*connect to the server*/
    if(connect(socketfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
        perror("connect() error");
		exit(1);
	}

	return socketfd;
}

char* createCMD(char* cmd, char* value){
	char* command = (char*)malloc(sizeof(char)*MAX_SIZE);

	if(sprintf(command, "%s %s\r\n", cmd, value) < 0){
		perror("Sprintf failed");
		return NULL;
	}

	return command;
}

//Returns the respective message for a command
int receivedMessage(int socketFD, char *buffer, int size) {
    int numberBytes;

    numberBytes = recv(socketFD, buffer, size, 0);
    if (numberBytes != size) {
		//Termination flag
        buffer[numberBytes] = '\0';
    }

    return numberBytes;
}

int FTPlogin(int socketFD, char* username, char* password) {
    char buffer[MAX_SIZE];

    printf("Username: %s\n", username);
    printf("Password: %s\n", password);

	//sending the username and waiting for a response
    char* userCMD = createCMD(USER, username);

    printf("Username CMD: %s\n", userCMD);
    printf("Sending username...\n");

    if (write(socketFD, userCMD, strlen(userCMD)) == -1) {
        perror("Error sending username");
        exit(1);
    }


	if (receivedMessage(socketFD, buffer, MAX_SIZE) == -1) {
		perror("Error receiving the respective message -> RECV error");
        exit(1);
    }

    printf("%s\n", buffer);

    if(strncmp(buffer, "230", 3) != 0 && strncmp(buffer, "331", 3) != 0){
	perror("Message received with the wrong code");
	exit(1);
    }

	//if the username was correct we need to send the password
    char* passCMD = createCMD(PASSWORD, password);

    printf("Password CMD: %s\n", passCMD);
    printf("Sending password...\n");

    if (write(socketFD, passCMD, strlen(passCMD)) == -1) {
        perror("Error sending password");
        exit(1);
    }

    if(receivedMessage(socketFD, buffer, MAX_SIZE) == -1){
	perror("Error receiving the respective message -> RECV error");
        exit(1);
    }

    printf("%s\n", buffer);

    if(strncmp(buffer, "230", 3) != 0 && strncmp(buffer, "202", 3) != 0){
	perror("Message received with the wrong code");
	exit(1);
    }

    return 0;
}

int FTPpasv(int socketFD, char* address) {
    char buffer[MAX_SIZE];
    int ip1, ip2, ip3, ip4, port1, port2;

    printf("Entering passive mode...\n");

    char *pasvCMD = createCMD(PASV, "");

    if (write(socketFD, pasvCMD, strlen(pasvCMD)) == -1) {
        perror("Error with the passive mode");
        exit(1);
    }

    if (receivedMessage(socketFD, buffer, MAX_SIZE) == -1) {
	perror("Error receiving the respective message -> RECV error");
        exit(1);
    }

    printf("%s\n", buffer);

    if(strncmp(buffer, "227", 3) != 0){
	perror("Message received with the wrong code");
	exit(1);
    }

    if(sscanf(buffer, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d).\n", &ip1, &ip2, &ip3, &ip4, &port1, &port2) != 6){
			perror("sscanf error");
			exit(1);
		}

    if(sprintf(address, "%d.%d.%d.%d", ip1, ip2, ip3, ip4) < 0){
			perror("sprintf error");
			exit(1);
		}

    int port = (256 * port1) + port2;

    return port;
}

int FTPret(int socketFD, char* path) {
    char buffer[MAX_SIZE];

    char *retCMD = createCMD(RETR, path);

    printf("Retrieving file...\n");

    if (write(socketFD, retCMD, strlen(retCMD)) == -1) {
	perror("Error retrieving file");
	exit(1);
    }

    if (receivedMessage(socketFD, buffer, MAX_SIZE) == -1) {
	perror("Error receiving the respective message -> RECV error");
        exit(1);
    }

    printf("%s\n", buffer);

    if(strncmp(buffer, "150", 3) != 0){
	perror("Message received with the wrong code");
	exit(1);
    }

    return 0;
}

int FTPlogout(int socketFD){
	char buffer[MAX_SIZE];

    char* quitCMD = createCMD(QUIT, "");

    printf("Quitting...\n");

    if (write(socketFD, quitCMD, strlen(quitCMD)) == -1) {
	perror("Error retrieving stopping the connection");
	exit(1);
    }

    if (receivedMessage(socketFD, buffer, MAX_SIZE) == -1) {
		perror("Error receiving the respective message -> RECV error");
        exit(1);
    }

    printf("%s\n", buffer);

    if(strncmp(buffer, "226", 3) != 0){
	perror("Message received with the wrong code");
	exit(1);
    }

    if(close(socketFD) == -1) {
	perror("Error closing socket");
	return 1;
    }

    return 0;
}

int FTPdownload(int socketFD, urlInfo* infoUrl) {
    int port, dataSocket, fileDescriptor, readBytes;
    char bufferMsg[MAX_SIZE];
    char address[MAX_SIZE];

    port = FTPpasv(socketFD, address);

    printf("IP address: %s\nPort: %d\n\n", address, port);

		dataSocket = connectSocket(address, port);
    if (dataSocket == -1) {
		//connectSocket has a perror already in case of error
         exit(-1);
    }

    if (FTPret(socketFD, infoUrl->pathname) == -1) {
		//FTPret has a perror already in case of error
        exit(-1);
    }

    if ((fileDescriptor = open(infoUrl->filename, O_CREAT | O_WRONLY | O_TRUNC, MODE)) == -1) // MODE = 0666
		{
        perror(infoUrl->filename);
        exit(-1);
    }

    while ( (readBytes = read(dataSocket, bufferMsg, MAX_SIZE)) > 0 ) {
        if (write(fileDescriptor, bufferMsg, readBytes) < 0) {
            perror("Error writing to file");
            exit(-1);
        }
    }

		close(fileDescriptor);
    return FTPlogout(socketFD);
}

int main(int argc, char** argv) {
    char *IP, message[MAX_SIZE];
    int socketFD;
    urlInfo *info = malloc(sizeof(urlInfo));

    if ((info = parser(argv[1])) == NULL) {
				perror("Error parsing URL");
        exit(1);
    }

    printf("\nUser: %s\nPassword: %s\nHost: %s\nPath: %s\nFile name: %s\n",
					info->name, info->password,
					info->host, info->pathname, info->filename);

    if ((IP = getHostIP(info->host)) == NULL ) {
				perror("Error getting IP address");
        exit(1);
    }

    printf("\nIP address: %s\n\n", IP);

    printf("Connecting to server...\n");
    if ( (socketFD = connectSocket(IP, PORT)) == -1) {
		//connectSocket has already a error message
        exit(1);
    }

    if (receivedMessage(socketFD, message, MAX_SIZE) == -1) {
		perror("Error receiving message");
        exit(1);
    }

    printf("%s\n", message);

    if (FTPlogin(socketFD, info->name, info->password)) {
		//FTPlogin has already a error message
        exit(-1);
    }

    return FTPdownload(socketFD, info);
}
