#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#define FTP 							"ftp://"				/** @brief The beggining of URL */

#define FTP_SIZE 					6 							/** @brief FTP's size */

#define STR_CHAR_SIZE 		2 							/** @brief Char's size */

#define STR_MAX_LEN				256							/** @brief Maximum length of string */

#define DEFAULT_USER 			"anonymous" 		/** @brief default user */
#define DEFAULT_PASSWORD 	"mail@domain" 	/** @brief default password */

/**
	@brief Struct to save the information provided by the URL
*/
typedef struct {
	char* name;				/** @brief User's name */
	char* password;		/** @brief User's password */
	char* pathname;		/** @brief Path's name */
	char* filename;		/** @brief File's name */
	char* host;				/** @brief Host's name */
} urlInfo;

/**
	@brief This function parses the respective URL
	@param url URL to be parsed
	@return The struct with the URL's information
*/
urlInfo* parser(char* url);
