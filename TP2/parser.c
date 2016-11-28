#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#define FTP 			"ftp://"
#define FTP_SIZE 		6

#define STR_CHAR_SIZE 		2

#define DEFAULT_USER 		"anonymous"
#define DEFAULT_PASSWORD 	"mail@domain"

#define PORT 21

typedef struct {
	char* name;
	char* password;
	char* pathname;
	char* filename;
	char* host;
} urlInfo;

urlInfo* parser(char * url){
	urlInfo *info = malloc(sizeof(urlInfo));
	const char stop[STR_CHAR_SIZE] = ":";
	const char at[STR_CHAR_SIZE] = "@";
	const char bar[STR_CHAR_SIZE] = "/";

	if(strncmp(url, FTP, FTP_SIZE) != 0) {
		perror("ftp:// does not exist");
		return NULL;
	}
	
	if(strcmp(FTP, strtok(url, at)) == 0) {
		strcpy(info->name, DEFAULT_USER);
		strcpy(info->password, DEFAULT_PASSWORD);
	}

	else {
		memmove(url, url+FTP_SIZE, strlen(url)-FTP_SIZE);
		info->name = strtok(url, stop);

		memmove(url, url+strlen(info->name), strlen(url)-strlen(info->name));
		info->password = strtok(url, at);
	}

	memmove(url, url+strlen(info->password), strlen(url)-strlen(info->password));
	info->host = strtok(url, bar);

	memmove(url, url+strlen(info->host), strlen(url)-strlen(info->host));
	info->pathname = strtok(url, bar);
	
	memmove(info->filename, url+strlen(info->pathname), strlen(url)-strlen(info->pathname));

	return info;
}
