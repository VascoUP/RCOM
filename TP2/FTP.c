#include <netdb.h> 
#include <stdlib.h> 
#include <arpa/inet.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include <string.h>
#include <libgen.h>
#include <fcntl.h>
#include <errno.h>

char *getHostIP(char* host){
	struct hostent *h = gethostbyname(host);
	if (h == NULL) {  
		herror("gethostbyname");
		return NULL;
	}
	
	char *IP = inet_ntoa(*((struct in_addr *)(h->h_addr)));
	return IP;
}

int connectSocket(int *socketfd, char* IP, int port)
{
	/*server address handling*/
	bzero((char*)&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(IP);	/*32 bit Internet address network byte ordered*/
	server_addr.sin_port = htons(port);				/*server TCP port must be network byte ordered */
	
	/*open an TCP socket*/
	if ((*socketfd = socket(AF_INET,SOCK_STREAM,0)) < 0) 
	{
    	perror("socket");
        return 1;
    }
	
	/*connect to the server*/
    if(connect(*socketdd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
        perror("connect");
		return 1;
	}
	
	return 0;
}
