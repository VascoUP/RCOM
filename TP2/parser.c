#include "parser.h"

int parser(char * url, urlInfo info){

	const char stop[STR_CHAR_SIZE] = ":";
	const char at[STR_CHAR_SIZE] = "@";
	const char bar[STR_CHAR_SIZE] = "/";

	//To see if the url begins with ftp://
	if(strncmp(url, FTP, FTP_SIZE) != 0) {
		perror("ftp:// does not exist");
		return 1;
	}

	//To obtain the username and password
	if(strlen(strtok(url, at)) == strlen(url)) {
		info.name = DEFAULT_USER;
		info.password= DEFAULT_PASSWORD;

		printf("%s - %s\n", info.name, info.password);
	}

	else {
		memmove(url, url+FTP_SIZE, strlen(url)-FTP_SIZE);
		info.name = strtok(url, stop);

		memmove(url, url+strlen(info.name), strlen(url)-strlen(info.name));
		info.password = strtok(url, at);

		printf("%s - %s\n", info.name, info.password);
	}

	memmove(url, url+strlen(info.password), strlen(url)-strlen(info.password));
	info.host = strtok(url, bar); //To obtain the host name


	memmove(url, url+strlen(info.host), strlen(url)-strlen(info.host));
	info.pathname = strtok(url, bar); //To obtain the path name

	memmove(info.filename, url+strlen(info.pathname), strlen(url)-strlen(info.pathname)); //To obtain the file name

	printf("%s - %s -%s\n", info.host, info.pathname, info.filename);

	return 0;
}
