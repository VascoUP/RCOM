#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "parser.h"

#define MAX_SIZE 	256			/** @brief Buffer's maximum size */

#define USER 		"USER "		/** @brief User's macro */
#define PASSWORD 	"PASS "		/** @brief Password's macro */
#define PASV 		"PASV "     /** @brief Passive mode's macro */
#define RETR 		"RETR "     /** @brief Retrieve's macro */
#define RETR 		"QUIT "		/** @brief Quit's macro */

#define MODE		0777		/** @brief Oppening file's mode */
#define PORT 		21			/** @brief Port's number */

/**
	@brief Struct with information to be used in the functions
*/
typedef struct{
	int port;		/** @brief Port's number */
	char* address;	/** @brief Address used */
}info;

/**
	@brief Function to get the host's IP address
	@param host Host of the connection's address
	@return The respective IP address
*/
char *getHostIP(char* host);

/**
	@brief It connects the sockets
	@param IP IP address
	@param port Port number used to connect the sockets
	@return The socket's descriptor or 1 if error
*/
int connectSocket(char* IP, int port);

/**
	@brief It creates the commands
	@param cmd Command name
	@param value Value to use with the command
	@return The command created
*/
char* createCMD(char* cmd, char* value);

/**
	@brief Function that verifies the message received with the function recv()
	@param socketFD Socket's descriptor
	@param buffer Buffer where the command is placed
	@param size Buffer's size
	@return Bytes' number returned by recv() function
*/
int receivedMessage(int socketFD, char *buffer, int size);

/**
	@brief It makes the login to do the download
	@param socketFD Socket's descriptor
	@param username User's name to be used in the login
	@param password User's password	to be used in the login
	@return 0 if everything is correct, 1 if it doesn't
*/
int FTPlogin(int socketFD, char* username, char* password);

/**
	@brief Function which implements the passive mode
	@param socketFD Socket's descriptor
	@return A struct with port's number and address information
*/
info* FTPpasv(int socketFD);

/**
	@brief Function which implements the retrieve command
	@param socketFD Socket's descriptor
	@param path Path used
	@return 0 if everything is correct, 1 if it doesn't
*/
int FTPret(int socketFD, char* path);

/**
	@brief It makes the logout
	@param socketFD Socket's descriptor
	@return 0 if everything is correct, 1 if it doesn't
*/
int FTPlogout(int socketFD);

/**
	@brief It makes the download and after it makes the logout
	@param socketFD Socket's descriptor
	@param info Struct with the URL's information
	@return 0 if everything is correct, 1 if it doesn't
*/
int FTPdownload(int socketFD, urlInfo info);

/**
	@brief This is the main function
	@param argc Number of arguments
	@param argv Array with the arguments
	@return 0 if everything is correct, 1 if it doesn't
*/
int main(int argc, char** argv);