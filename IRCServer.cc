
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
#include <iostream>
//#include <vector.h>
#define MAX_USERS 1000

#include "IRCServer.h"
#include "HashTableVoid.h"
//using namespace std;
FILE * file;
char * uname(char * userpass);
void write_client(int fd, char * string);
int m = 100; int n = 100;
int number_rooms = 0;
char * userpass[100];
int number_users = 0;

struct Room {
	const char * room_name = (char *) malloc(sizeof(char) * 100);
	char * users_in_room[100]; // = (char*)malloc(sizeof(char) * 100); //uses userpass
	char * messages[5]; // = (char*)malloc(sizeof(char) * 1000);
	int msg_num = 0;
	int number_users_room = 0;
};

Room rooms[100];

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
	else if (!strcmp(command, "CREATE-ROOM")) {
		createRoom(fd,user,password,args);
	}
	else if (!strcmp(command, "LIST-ROOMS")) {
		listRoom(fd,user,password);
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
	char * string = (char*)malloc(100 * sizeof(char));
	file = fopen("password.txt","r");
	int j = 0;

	//Initialize userpass
	while(j < 100) {
		userpass[j] = (char*) malloc(sizeof(char) * 100);
		//	*userpass[j] = "";
		j++;
	}

	//Initialize Room array
	int k = 0; int l = 0; int m = 0;
	while(k < 100) {
		while (l < 100) {
			rooms[k].users_in_room[l] = (char*) malloc(sizeof(char) * 100);
			l++;
		}
		while(m < 5) {

			rooms[k].messages[m] = (char*) malloc(sizeof(char) * 100);
			m++;
		}
		k++;
	}

	int i = 0;
	// Open password file
	if(file != NULL) {
		while((string = nextword(file)) != NULL) {
			strcpy(userpass[i],string);
			printf("userpass %s\n",userpass[i]);
			i++;
			number_users++;
		}

	}		

	// Initialize users in room

	// Initalize message list
}


char * nyancat(const char * user, const char * password) {
	char * s = (char *) malloc(sizeof(char) * 100);
	strcpy(s,user);
	strcat(s,"^");
	strcat(s,password);
	char * m = (char *) malloc(sizeof(char) * 100);
	m = strdup(s);
	return m;
}

bool
IRCServer::checkPassword(int fd, const char * user, const char * password) {
	// Here check the password
	int i = 0;

	char * s = (char *) malloc(sizeof(char) * 100);

	while(i < number_users) {
		printf("userpass_checks %s Entered %s\n",userpass[i],nyancat(user,password));

		if(strcmp(userpass[i],nyancat(user,password)) == 0) {
			return true;
		}
		i++;
	}

	char * msg = "PASSWORD INCORRECT\r\n";
	write(fd,msg,strlen(msg));
	return false;

}

void IRCServer::addUser(int fd, const char * user, const char * password, const char * args)
{
	int i = 0;
	file = fopen("password.txt","a");
	//Rooms
	while(i < number_users) {
		if(strcmp(uname(userpass[i]),user) == 0) {
			const char * m =  "DENIED\r\n";
			write(fd, m, strlen(m));
			return;
		}
		i++;
	}

	userpass[number_users] = nyancat(user,password);
	number_users++;
	//Testing
	int j = 0;
	/*while(j < number_users) {
	  printf("Username %s\n",userpass[j]);
	  j++;
	  }*/
	fprintf(file,"%s^%s\n",user,password);	
	fclose(file);
	const char * msg =  "OK\r\n";
	write(fd, msg, strlen(msg));
	return;


}

	void
IRCServer::createRoom(int fd, const char * user, const char * password, const char * args)
{
	int i = 0;
	//write(fd,"yolo1",5);
	if(checkPassword(fd,user,password) == true) {

		//Room created before, check
		while(i < number_rooms) {
			if(strcmp(args,rooms[i].room_name) == 0) {
				const char * s = "DENIED ROOM CREATION\r\n";
				write(fd,s,strlen(s));
				printf(">>>>>Server Message>>>>>>\n");
				printf("DENIED ROOM CREATION\r\n Total rooms %d\n",number_rooms);
				return;
			}
			i++;
		}	

		//Creating room
		printf("Entered room creation, Total Rooms %d\n",number_rooms);
		if(number_rooms < 100) {
			rooms[number_rooms].room_name = strdup(args); 									
			const char * s = "OK, ROOM CREATED\r\n";
			printf(">>>>>Server Message>>>>>>\n");
			write(fd,s,strlen(s));
			printf("Room Name %s, Room Number %d\n",rooms[number_rooms].room_name,number_rooms);
			number_rooms++;			
			return;
		}
	}

	return;
}

char * uname(char * userpass) {
	char * un = (char*)malloc(sizeof(char) * 200);
	char * token = (char*)malloc(sizeof(char) * 200);
	un = strdup(userpass);
	token = strdup(strtok(un,"^"));
	return token;
}

	void
IRCServer::listRoom(int fd, const char * user, const char * password)
{	
	int i = 0;
	char * heading = "######## LISTING ROOMS ########\r\n";
	char * s = (char*)malloc(sizeof(char) * 1000);


	if(checkPassword(fd,user,password) == true) {
		
		while(i < number_rooms) {
			if(i == 0) {
				strcpy(s,rooms[i].room_name);
			}
			else {
				strcat(s,rooms[i].room_name);
			}
			strcat(s,"\r\n");
			printf("room name %s\n",rooms[i].room_name);
			i++;
		}
		write_client(fd,heading);
		write_client(fd,s);
	}
	return;

} 
//________________________________________________________________________________________________COULD CAUSE ERROR ON MALLOC____________________________________________________________________________
void write_client(int fd, char * string) {
        const char * s = (char*)malloc(sizeof(char) * number_users * 100);
	s = strdup(string);
	write(fd,s,strlen(s));
	return;
}

	void
IRCServer::enterRoom(int fd, const char * user, const char * password, const char * args)
{	
	if(number_rooms == 0) {
		write_client(fd,"NO ROOMS CREATED YET\r\n");
		return;
	}

	int check = 0; int i = 0;
	if(checkPassword(fd,user,password) == true) {
		while(i < number_rooms) {
			if(strcmp(rooms[i].room_name,args) == 0) {
				check = 1;
				rooms[i].users_in_room[rooms[i].number_users_room]  = strdup(nyancat(user,password));
				rooms[i].number_users_room++;
				write_client(fd,"ROOM ENTERED\r\n");
				return;
			}
			i++;
		}

		if(check == 0) {
			write_client(fd,"NO SUCH ROOM FOUND\r\n");
		}
	}
	return;
}
	void
IRCServer::leaveRoom(int fd, const char * user, const char * password, const char * args)
{
	if(checkPassword(fd,user,password) == true) {
	int i = 0; int check1 = 0; int l = 0; int check2 = 0;
	while(i < number_rooms) {
		if(strcmp(rooms[i].room_name,args) == 0) {
			check1 = 1;
			while(l < rooms[i].number_users_room) {
				if(strcmp(uname(rooms[i].users_in_room[l]),user) == 0) {
					check2 = 1;
					
					break;
				}
				l++;
			}
		break;
		}
		i++;
	}

	if(check1 == 0) {
		write_client(fd,"NO SUCH ROOM");
	}
	if(check2 == 0) {
		write_client(fd,"NO SUCH USER IN ROOM");
	}

	if(check1 == 1 && check2 == 1) {
		while(l < rooms[i].number_users_room - 1) {
			rooms[i].users_in_room[l] = rooms[i].users_in_room[l + 1];
			l++;

		}
		rooms[i].number_users_room--;
	}
	return;
	}
	return;

}

	void
IRCServer::sendMessage(int fd, const char * user, const char * password, const char * args)
{
	//If SEGV CHECK INIT________________________________________________________________________________________
	if(checkPassword(fd,user,password) == true) {	
		
			char * token = (char *) malloc(sizeof(char) * 100);
			char args_cpy[100] = "";
			
			strcpy(args_cpy,args);
			token = strtok(args_cpy," ");
		
			char * s = strdup(args);
			s += strlen(token) + 1;
		
			int i = 0;
			int check = 0;
					
			while(i < number_rooms) {
				if(strcmp(rooms[i].room_name,token) == 0) {
					check  = 1;			
					break;
				}		
				i++;	
			}
			
			if(check == 0) {write_client(fd,"NO SUCH ROOM\r\n"); return;}
			
			if(strcmp(s,"") == 0) {
				write_client(fd,"ENTER MESSAGE\r\n");				
				return;
			}

			if(rooms[i].msg_num == 5) {
			
				int j = 0;
				while(j < rooms[i].msg_num - 1) {
					rooms[i].messages[j] = rooms[i].messages[j + 1];
					j++;
				}
						
				rooms[i].messages[rooms[i].msg_num - 1] = strdup(s);
				//rooms[i].msg_num++;______________________________________________________________________ISSUE?
				write_client(fd,"MESSAGE SENT!\r\n");
			}
			else {
				rooms[i].messages[rooms[i].msg_num] = strdup(s);
				rooms[i].msg_num++;	
				write_client(fd,"MESSAGE SENT!\r\n");
			}
	}

}

void

IRCServer::getMessages(int fd, const char * user, const char * password, const char * args)
{
	if(checkPassword(fd,user,password) == true) {	
	
		write_client(fd,"~~~~~~~~~~~~~~~~~``LIST OF MESSAGES``~~~~~~~~~~~~~~~~\r\n");		
		int i = 0; int check = 0;
		printf("ABOVE ROOM CHECKS\n");		
		while(i < number_rooms) {
			if(strcmp(rooms[i].room_name,args) == 0) {
				check  = 1;			
				break;
			}		
			i++;	
		}
		
		if(check == 0) {write_client(fd,"NO SUCH ROOM\r\n");}
		printf("ROOMS CHECKED\n");
		char * messages_list = (char*) malloc(sizeof(char) * rooms[i].msg_num * 200);
		printf("Blw msg_list, abv strcpy\n");
		strcpy(messages_list,"");
		printf("Abv for loop, blw strcpy\n");
		for(int j = 0; j < rooms[i].msg_num; j++) {
			strcat(messages_list,rooms[i].messages[j]);
			strcat(messages_list,"\r\n");
		}	
	
		write_client(fd,messages_list);

	}
}

	void
IRCServer::getUsersInRoom(int fd, const char * user, const char * password, const char * args)
{
	if(checkPassword(fd,user,password)) {

		if(number_rooms == 0) {
			write_client(fd,"NO ROOMS CREATED YET\r\n");
		}

		int check = 0; int i = 0; int j = 0;
		while(i < number_rooms) {
			if(strcmp(args,rooms[i].room_name) == 0) {
				check = 1;
				break;
			}
			i++;
		}
		if(check == 0) {
			write_client(fd,"NO SUCH ROOM CREATED\r\n");
		}

		if(check == 1) {

			char * temp = (char*)malloc(sizeof(char) * 200);

		for(int p = 0; i < number_users; i++) {
			for(int q = p + 1; j < number_users - 1; j++) {
				if(strcmp(uname(rooms[i].users_in_room[p]),uname(rooms[i].users_in_room[q])) > 0) {
					temp = rooms[i].users_in_room[p];
					rooms[i].users_in_room[p] = rooms[i].users_in_room[q];
					rooms[i].users_in_room[q] = temp;
				}
			}
		}
			
			char * users_in_r = (char*)malloc(sizeof(char) * 10000);
			char * heading = (char*)malloc(sizeof(char) * 50);
			sprintf(heading,"USERS IN ROOM: %s\n", rooms[i].room_name);
			strcpy(users_in_r,heading);

			if(rooms[i].number_users_room == 0) {
				write_client(fd,"NO USERS IN THIS ROOM\r\n");
			}

			while(j < rooms[i].number_users_room) {
				strcat(users_in_r,uname(rooms[i].users_in_room[j]));
				strcat(users_in_r,"\r\n");
				j++;	
			}

			write_client(fd,users_in_r);
		}
	}
}	


	void
IRCServer::getAllUsers(int fd, const char * user, const char * password,const  char * args)
{
	if(checkPassword(fd,user,password)) {
		char * temp = (char*)malloc(sizeof(char) * 200);

		for(int i = 0; i < number_users; i++) {
			for(int j = i + 1; j < number_users - 1; j++) {
				if(strcmp(uname(userpass[i]),uname(userpass[j])) > 0) {
					temp = userpass[i];
					userpass[i] = userpass[j];
					userpass[j] = temp;
				}
			}
		}

		char * print = (char*) malloc(sizeof(char) * 200 * number_users);

		int k = 0;
		while(k < number_users) {
			if(k == 0) {
				strcpy(print,userpass[k]);
			}
			else {
				strcat(print,userpass[k]);
			}
			strcat(print,"\r\n");
			k++;
		} 
		write_client(fd,print);
	}
}

