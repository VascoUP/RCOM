#include "parser.h"

urlInfo* parser(char * url){
	urlInfo *info = malloc(sizeof(urlInfo));
	const char stop[STR_CHAR_SIZE] = ":";
	const char at[STR_CHAR_SIZE] = "@";
	const char bar[STR_CHAR_SIZE] = "/";
	char current[strlen(url)];
	strcpy(current, url);

	//To see if the url begins with ftp://
	if(strncmp(url, FTP, FTP_SIZE) != 0) {
		perror("ftp:// does not exist");
		return NULL;
	}
	//To obtain the username and password
	if(strlen(strtok(current, at)) == strlen(url)) {
		info->name = DEFAULT_USER;
		info->password= DEFAULT_PASSWORD;

		memmove(url, url+FTP_SIZE, strlen(url)-FTP_SIZE);
		url[strlen(url)-FTP_SIZE] = 0;

		strcpy(current, url);

		// Host
		info->host = malloc( STR_MAX_LEN );
		strcpy(info->host, strtok(current, bar));

		memmove(url, url+(strlen(info->host)+1), strlen(url)-(strlen(info->host)+1)); //Mais 1 e menos 1 para contar com a barra
		url[strlen(url)-(strlen(info->host)+1)] = 0;

		strcpy(current, url);

		// Pathname
		info->pathname = malloc( STR_MAX_LEN );
		strcpy(info->pathname, strtok(current, bar)); //To obtain the path name

		memmove(url, url+(strlen(info->pathname)+1), strlen(url)-(strlen(info->pathname)-1)); //To obtain the file name

		strcpy(current, url);

		// Rest of the pathname until the file name
		char* strRes = strtok(current, bar);
		int resLen = strlen(strRes);

		while(resLen != strlen(url)) {
			strcat(info->pathname, bar);
			strcat(info->pathname, strRes); //To obtain the path name

			memmove(url, url+(resLen+1), strlen(url)-(resLen-1)); //To obtain the file name
			strcpy(current, url);

			strRes = strtok(current, bar);
			resLen = strlen(strRes);
		}

		info->filename = url;
	}

	else {

		memmove(url, url+FTP_SIZE, strlen(url)-FTP_SIZE);
		url[strlen(url)-FTP_SIZE] = 0;

		strcpy(current, url);

		// Name
		info->name = malloc( STR_MAX_LEN );
		strcpy(info->name, strtok(current, stop));

		memmove(url, url+strlen(info->name), strlen(url)-strlen(info->name)); //Mais 1 e menos 1 para contar com a barra
		url[strlen(url)-strlen(info->name)] = 0;

		strcpy(current, url);

		// Password
		info->password = malloc( STR_MAX_LEN );
		strcpy(info->password, strtok(current, at));

		memmove(url, url+strlen(info->password)+1, strlen(url)-strlen(info->password)-1); //Mais 1 e menos 1 para contar com a barra
		url[strlen(url)-strlen(info->password)-1] = 0;

		strcpy(current, url);

		// Host
		info->host = malloc( STR_MAX_LEN );
		strcpy(info->host, strtok(current, bar));

		memmove(url, url+(strlen(info->host)+1), strlen(url)-(strlen(info->host)+1)); //Mais 1 e menos 1 para contar com a barra
		url[strlen(url)-(strlen(info->host)+1)] = 0;

		strcpy(current, url);

		// Pathname
		info->pathname = malloc( STR_MAX_LEN );
		strcpy(info->pathname, strtok(current, bar)); //To obtain the path name

		memmove(url, url+(strlen(info->pathname)+1), strlen(url)-(strlen(info->pathname)-1)); //To obtain the file name

		strcpy(current, url);

		// Rest of the pathname until the file name
		char* strRes = strtok(current, bar);
		int resLen = strlen(strRes);

		while(resLen != strlen(url)) {
			strcat(info->pathname, bar);
			strcat(info->pathname, strRes); //To obtain the path name

			memmove(url, url+(resLen+1), strlen(url)-(resLen-1)); //To obtain the file name
			strcpy(current, url);

			strRes = strtok(current, bar);
			resLen = strlen(strRes);
		}

		info->filename = url;
	}

	strcat(info->pathname, bar);
	strcat(info->pathname, info->filename);
	
	return info;
}
