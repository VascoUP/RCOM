#include "parser.h"

urlInfo* parser(char * url){
	urlInfo *info = malloc(sizeof(urlInfo));
	//const char stop[STR_CHAR_SIZE] = ":";
	const char at[STR_CHAR_SIZE] = "@";
	const char bar[STR_CHAR_SIZE] = "/";
	char current[strlen(url)];
	strcpy(current, url);
	printf("1 - URL : %s\n", url);
	printf("1 - URL COPY : %s\n", current);

	//To see if the url begins with ftp://
	if(strncmp(url, FTP, FTP_SIZE) != 0) {
		perror("ftp:// does not exist");
		return NULL;
	}
	//To obtain the username and password
	if(strlen(strtok(current, at)) == strlen(url)) {
		printf("2 - URL: %s\n", url);
		info->name = DEFAULT_USER;
		info->password= DEFAULT_PASSWORD;

		printf("\nUSER: %s - PASSWORD: %s\n\n", info->name, info->password);

		memmove(url, url+FTP_SIZE, strlen(url)-FTP_SIZE);
		printf("3 - URL: %s\n", url);
		strcpy(current, url);
		printf("3 - URL COPY: %s\n", current);
		info->host = strtok(current, bar);
		printf("\nHOST: %s\n\n", info->host);

		memmove(url, url+(strlen(info->host)+1), strlen(url)-(strlen(info->host)+1)); //Mais 1 e menos 1 para contar com a barra
		printf("\nHOST2: %s\n\n", info->host);
		printf("4 - URL: %s\n", url);
		strcpy(current, url);
		printf("4 - URL COPY: %s\n", current);

		printf("\nHOST2: %s\n\n", info->host);
		info->pathname = strtok(current, bar); //To obtain the path name
		printf("\nPATH: %s\n\n", info->pathname);

		printf("\nHOST2: %s\n\n", info->host);

		memmove(url, url+(strlen(info->pathname)+1), strlen(url)-(strlen(info->pathname)-1)); //To obtain the file name
		printf("5 - URL: %s\n", url);
		info->filename = url;
		printf("\nFILE: %s\n\n", info->filename);
	}

	else {
		/*memmove(url, url+FTP_SIZE, strlen(url)-FTP_SIZE);
		strcpy(current, url);
		info->name = strtok(current, stop);

		memmove(url, url+strlen(info->name), strlen(url)-strlen(info->name));
		strcpy(current, url);
		info->password = strtok(current, at);

		memmove(url, url+strlen(info->password), strlen(url)-strlen(info->password));
		strcpy(current, url);
		info->host = strtok(current, bar);

		memmove(url, url+strlen(info->host), strlen(url)-strlen(info->host));
		strcpy(current, url);
		info->pathname = strtok(current, bar); //To obtain the path name

		memmove(info->filename, url+strlen(info->pathname), strlen(url)-strlen(info->pathname)); //To obtain the file name
*/
	}

	printf("User: %s - Pass: %s - Host: %s - Path: %s - File: %s\n", info->name, info->password, info->host, info->pathname, info->filename);
	return info;
}
