
#ifndef IRC_SERVER
#define IRC_SERVER

#define PASSWORD_FILE "password.txt"

class IRCServer {
	// Add any variables you need

	private:
		int open_server_socket(int port);

	public:
		void initialize();
		//My Functions
		//void extract_from_CommandLine(char cmdLine[], char * &  cmd, char * &  usr, char * &  pswrd);
		//Given Functions
		/*struct s_users {
			char * s_username;
			char * s_password;
		};*/

		//struct s_users users[1000];

		void createRoom(int fd, const char * user, const char * password, const char * args);
		void listRoom(int fd, const char * user, const char * password);
		bool checkPassword(int fd, const char * user, const char * password);
		//int find_s_users(struct s_users user_array[1000],const char * user);
		void init_users(char * userpass[100]);
		char * nextword(FILE * fd);
		void processRequest( int socket );
		void addUser(int fd, const char * user, const char * password, const char * args);
		void sign_in(int fd, const char * user, const char * password, const char * args);
		void enterRoom(int fd, const char * user, const char * password, const char * args);
		void leaveRoom(int fd, const char * user, const char * password, const char * args);
		void sendMessage(int fd, const char * user, const char * password, const char * args);
		void getMessages(int fd, const char * user, const char * password, const char * args);
		void getUsersInRoom(int fd, const char * user, const char * password, const char * args);
		void getAllUsers(int fd, const char * user, const char * password, const char * args);
		void runServer(int port);
};

#endif
