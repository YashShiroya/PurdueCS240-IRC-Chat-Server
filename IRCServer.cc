
const char * usage =
"                                                               \n"
"IRCServer:                                                   \n"
"                                                               \n"
"Simple server program used to communicate multiple users       \n"
"                                                               \n"
"To use it in one window type:                                  \n"
"                                                               \n"
"   IRCServer <port>                                          \n"
"                                                               \n"
"Where 1024 < port < 65536.                                     \n"
"                                                               \n"
"In another window type:                                        \n"
"                                                               \n"
"   telnet <host> <port>                                        \n"
"                                                               \n"
"where <host> is the name of the machine where talk-server      \n"
"is running. <port> is the port number you used when you run    \n"
"daytime-server.                                                \n"
"                                                               \n";

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#define MAX_USERS 1000

#include "IRCServer.h"
#include "HashTableVoid.h"

FILE * file;

struct s_users {
	char * s_username;
	char * s_password;
};
typedef s_users s_users;
 s_users users[MAX_USERS];
int QueueLength = 5;
//void extract_from_CommandLine(char * &  cmd, char * &  usr, char * &  pswrd);
int IRCServer::open_server_socket(int port) {

	// Set the IP address and port for this server
	struct sockaddr_in serverIPAddress; 
	memset( &serverIPAddress, 0, sizeof(serverIPAddress) );
	serverIPAddress.sin_family = AF_INET;
	serverIPAddress.sin_addr.s_addr = INADDR_ANY;
	serverIPAddress.sin_port = htons((u_short) port);

	// Allocate a socket
	int masterSocket =  socket(PF_INET, SOCK_STREAM, 0);
	if ( masterSocket < 0) {
		perror("socket");
		exit( -1 );
	}

	// Set socket options to reuse port. Otherwise we will
	// have to wait about 2 minutes before reusing the sae port number
	int optval = 1; 
	int err = setsockopt(masterSocket, SOL_SOCKET, SO_REUSEADDR, 
			(char *) &optval, sizeof( int ) );

	// Bind the socket to the IP address and port
	int error = bind( masterSocket,
			(struct sockaddr *)&serverIPAddress,
			sizeof(serverIPAddress) );
	if ( error ) {
		perror("bind");
		exit( -1 );
	}

	// Put socket in listening mode and set the 
	// size of the queue of unprocessed connections
	error = listen( masterSocket, QueueLength);
	if ( error ) {
		perror("listen");
		exit( -1 );
	}

	return masterSocket;
}

void IRCServer::runServer(int port)
{
	int masterSocket = open_server_socket(port);

	initialize();

	while ( 1 ) {

		// Accept incoming connections
		struct sockaddr_in clientIPAddress;
		int alen = sizeof( clientIPAddress );
		int slaveSocket = accept( masterSocket,
				(struct sockaddr *)&clientIPAddress,
				(socklen_t*)&alen);

		if ( slaveSocket < 0 ) {
			perror( "accept" );
			exit( -1 );
		}

		// Process request.
		processRequest( slaveSocket );		
	}
}

	int
main( int argc, char ** argv )
{
	// Print usage if not enough arguments
	if ( argc < 2 ) {
		fprintf( stderr, "%s", usage );
		exit( -1 );
	}

	// Get the port from the arguments
	int port = atoi( argv[1] );

	IRCServer ircServer;

	// It will never return
	ircServer.runServer(port);

}

//
// Commands:
//   Commands are started y the client.
//
//   Request: ADD-USER <USER> <PASSWD>\r\n
//   Answer: OK\r\n or DENIED\r\n
//
//   REQUEST: GET-ALL-USERS <USER> <PASSWD>\r\n
//   Answer: USER1\r\n
//            USER2\r\n
//            ...
//            \r\n
//
//   REQUEST: CREATE-ROOM <USER> <PASSWD> <ROOM>\r\n
//   Answer: OK\n or DENIED\r\n
//
//   Request: LIST-ROOMS <USER> <PASSWD>\r\n
//   Answer: room1\r\n
//           room2\r\n
//           ...
//           \r\n
//
//   Request: ENTER-ROOM <USER> <PASSWD> <ROOM>\r\n
//   Answer: OK\n or DENIED\r\n
//
//   Request: LEAVE-ROOM <USER> <PASSWD>\r\n
//   Answer: OK\n or DENIED\r\n
//
//   Request: SEND-MESSAGE <USER> <PASSWD> <MESSAGE> <ROOM>\n
//   Answer: OK\n or DENIED\n
//
//   Request: GET-MESSAGES <USER> <PASSWD> <LAST-MESSAGE-NUM> <ROOM>\r\n
//   Answer: MSGNUM1 USER1 MESSAGE1\r\n
//           MSGNUM2 USER2 MESSAGE2\r\n
//           MSGNUM3 USER2 MESSAGE2\r\n
//           ...\r\n
//           \r\n
//
//    REQUEST: GET-USERS-IN-ROOM <USER> <PASSWD> <ROOM>\r\n
//    Answer: USER1\r\n
//            USER2\r\n
//            ...
//            \r\n
//



void IRCServer::processRequest( int fd )
{
	// Buffer used to store the comand received from the client
	const int MaxCommandLine = 1024;
	char commandLine[ MaxCommandLine + 1 ];
	int commandLineLength = 0;
	int n;

	// Currently character read
	unsigned char prevChar = 0;
	unsigned char newChar = 0;

	//
	// The client should send COMMAND-LINE\n
	// Read the name of the client character by character until a
	// \n is found.
	//

	// Read character by character until a \n is found or the command string is full.
	while ( commandLineLength < MaxCommandLine &&
			read( fd, &newChar, 1) > 0 ) {

		if (newChar == '\n' && prevChar == '\r') {
			break;
		}

		commandLine[ commandLineLength ] = newChar;
		commandLineLength++;

		prevChar = newChar;
	}

	// Add null character at the end of the string
	// Eliminate last \r
	commandLineLength--;
	commandLine[ commandLineLength ] = 0;

	printf("RECEIVED: %s\n", commandLine);

	printf("The commandLine has the following format:\n");
	printf("COMMAND <user> <password> <arguments>. See below.\n");
	printf("You need to separate the commandLine into those components\n");
	printf("For now, command, user, and password are hardwired.\n");

	int a = 'a'; int i = 0; int space_encountered = 0; int j = 0;
	char cmd[20] = ""; char usr[20] =  ""; char pswrd[20]=  ""; char argz[20]= "";

	while((a = commandLine[i]) != '\0') {

		if(a == ' ') { 
			space_encountered++;
			if(space_encountered <= 3) {
				j = 0;
			}
		}
		else {
			if(space_encountered == 0) {
				cmd[j++] = a; cmd[j] = 0;
			}
			else if(space_encountered == 1) {
				usr[j++] = a; usr[j] = 0;
			}
			else if(space_encountered == 2) {
				pswrd[j++] = a; pswrd[j] = 0;
			}
		}
		if(space_encountered >= 3) {
			argz[j++] = a; argz[j] = 0;
		}
		i++;
	}

	if(strlen(argz) != 0) {
		int k = 0;

		while(k < strlen(argz) - 1) {
			argz[k] = argz[k+1];
			k++;
		}
		argz[k] = '\0';
	}

	const char * command = cmd;
	const char * user = usr;
	const char * password = pswrd;          
	const char * args = argz;


	printf("command=%s\n", command);
	printf("user=%s\n", user);
	printf( "password=%s\n", password );
	printf("args=%s\n", args);

	if (!strcmp(command, "ADD-USER")) {
		addUser(fd, user, password, args);
	}
	else if (!strcmp(command, "ENTER-ROOM")) {
		enterRoom(fd, user, password, args);
	}
	else if (!strcmp(command, "LEAVE-ROOM")) {
		leaveRoom(fd, user, password, args);
	}
	else if (!strcmp(command, "SEND-MESSAGE")) {
		sendMessage(fd, user, password, args);
	}
	else if (!strcmp(command, "GET-MESSAGES")) {
		getMessages(fd, user, password, args);
	}
	else if (!strcmp(command, "GET-USERS-IN-ROOM")) {
		getUsersInRoom(fd, user, password, args);
	}
	else if (!strcmp(command, "GET-ALL-USERS")) {
		getAllUsers(fd, user, password, args);
	}
	else {
		const char * msg =  "UNKNOWN COMMAND\r\n";
		write(fd, msg, strlen(msg));
	}

	// Send OK answer
	//const char * msg =  "OK\n";
	//write(fd, msg, strlen(msg));

	close(fd);	
}

#define MAXWORD 200
char word[MAXWORD];  

char * IRCServer::nextword(FILE * fd){
	int c;int i = 0;
	memset(word, 0, MAXWORD);
	while((c = fgetc(fd)) != EOF) {
		if(c != '\n') {
			word[i++] = c;
		}
		else if( c == '\n') {
			if(i > 0) {
				return word;
			}
		}
	}
	return NULL;
}



//My Functions____________________________
//Given Functions_________________________
	void
IRCServer::initialize()
{	
	char * string = (char*)malloc(20 * sizeof(char));
	file = fopen("userpass.txt","r");
	char * token = (char*)malloc(100 * sizeof(char));
	char sep[4] = "^";
	int number_users = 0;

	// Open password file
	if(file != NULL) {
		while((string = nextword(file)) != NULL) {
			token = strtok(string,sep);
			users[number_users].s_username = token;
			token = strtok(NULL,sep);
			users[number_users].s_password = token;
			number_users++;
		}
			
	}		
		
	// Initialize users in room

	// Initalize message list
}



/*struct s_users {
	char * s_username;
	char * s_password;
} 

typedef struct s_users s_users;*/

//s_users users[1000];



int IRCServer::init_s_users( s_users user_array[]) {
	int i = 0;
	while(i < MAX_USERS) {
		user_array[i].s_username = "default";
		user_array[i].s_password = "default";
		//i++;
	}
}

int IRCServer::find_s_users(s_users user_array[],const char * user) {
	int i = 0;
	while(i < MAX_USERS) {
		if(strcmp(user_array[i].s_username,user) == 0) {
			return 1;
		}
	}
	return -1;
}

bool
IRCServer::checkPassword(int fd, const char * user, const char * password) {
	// Here check the password
	return true;
}

//INITIALIZE
	/*int t = 0;
         while(t < MAX_USERS) {
                 user_array[t].s_username = "default";
                 user_array[t].s_password = "default";
		 t++;
		 
         }*/
//________________


void IRCServer::addUser(int fd, const char * user, const char * password, const char * args)
{
	file = fopen("userpass.txt","a+");
//	init_s_users(users);
//	while()
		if(file != NULL) {
			fprintf(file,"%s^%s\n",user,password);
		}

	fclose(file);
	const char * msg =  "OK\r\n";
	write(fd, msg, strlen(msg));

	return;
}

	void
IRCServer::createRoom(int fd, const char * user, const char * password, const char * args)
{

}

	void
IRCServer::listRoom(int fd, const char * user, const char * password, const char * args)
{

} 

	void
IRCServer::enterRoom(int fd, const char * user, const char * password, const char * args)
{

}
	void
IRCServer::leaveRoom(int fd, const char * user, const char * password, const char * args)
{
}

	void
IRCServer::sendMessage(int fd, const char * user, const char * password, const char * args)
{
}

	void
IRCServer::getMessages(int fd, const char * user, const char * password, const char * args)
{
}

	void
IRCServer::getUsersInRoom(int fd, const char * user, const char * password, const char * args)
{
}

	void
IRCServer::getAllUsers(int fd, const char * user, const char * password,const  char * args)
{

}

