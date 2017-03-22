/*
 *
 *   This is the Server for the Chat application running over sockets 
 *	 Professor are the categories of user who can start the chat room
 *   and Student are the user who can join them
 */

//Header files
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <unistd.h>
#include <crypt.h>
#include <time.h>

// This constant defines the maximum of clients to be connected to this Server
int static const MAX_CLIENTS = 200;
int static const MAX_NAME_SIZE = 50;
int static const MAX_USERNAME_SIZE = 50;
int static const MAX_CHATROOM_CLIENTS = 50;
int static const MAX_CHATROOM_QTY = 10;

// Structure to handle Clients Information
struct MACClient
{
	char name[50];
	char username[50];
	int socket;
	int chatRoom;
	int isProfessor;
};

// Structure to handle ChatRoom Information
struct ChatRoom
{
	char name[50];
	int number;
	struct MACClient professor;
	struct MACClient roomClients[50];
	int clientsQty;
};

// List of clients Sockets
int sockClients[200];
// Total of connected clients up to the moment
int qtyClients = 0;
// List of Chatroom
struct ChatRoom chatRoomList[10];
// Quantity of initialized ChatRooms up to the momment
int chatRoomQty = 0;
// Server IP Address
struct sockaddr_in server;
//char *SERVER_IP = "137.207.82.51";
char *SERVER_IP = "137.207.82.53";
int SOCKET_PORT = 0;

// Method signatures
void printServerRoomStatus();
void cleanOutDisconectedClient(struct MACClient *macClient);
struct MACClient* findClientBySocket(int socket);
int broadcastMessageToChatRoom(struct MACClient *clientFrom, char *msg);

//the thread heandler function to handle client connections
void *clientConnection_handler(void *);
void *clientConnection_handler1(void *);

/*
 *   Adds a new ChatRoom to the list of ChatRooms maintained by the Server
 *   Will look for AVAILABLE BLANK spots in the array that are NULL
 *   There is a maximum of MAX_CHATROOM_QTY maintained by the server.
 *   Returns 1 if it was succesfully added to the list and 0 if not.
 */
int addChatRoom(struct ChatRoom *newChatRoom) {
	int succesful = 0;
	if (chatRoomQty <= MAX_CHATROOM_QTY) {
		int i;
		for (i = 0; i < MAX_CHATROOM_QTY; i++) {
			if (chatRoomList[i].clientsQty <= 0) {
				chatRoomQty++;
				chatRoomList[i] = *newChatRoom;
				succesful = 1;
				break;
			}
		}
	}
	return succesful;
}

/*
 *   Create the Chat room by Professor's User Name
 *   Returns a pointer reference to the ChatRoom
 */
struct ChatRoom* createChatRoomByProfessor(struct MACClient *chatProfessor, char *roomName) {
	struct ChatRoom *room = NULL;
	int i;
	for (i = 0; i < MAX_CHATROOM_QTY; i++) {
		if (chatRoomList[i].clientsQty == 0) {
			chatRoomList[i].number = i;
			strcpy(chatRoomList[i].name,roomName);
			chatRoomList[i].professor = *chatProfessor;
			chatRoomList[i].professor.chatRoom = i;
			chatRoomList[i].roomClients[0] = *chatProfessor;
			chatRoomList[i].roomClients[0].chatRoom = i;
			chatRoomList[i].clientsQty = 1;
			chatRoomQty++;
			room = &chatRoomList[i];
			break;
		}
	}
	return room;
}

/* Remove Chat Rooms once professor logout*/
int removeChatRoom(char *profUser){
	int succesful=0;
	int i,j;
	for (i = 0; i < MAX_CHATROOM_QTY; i++) {
		if (strcmp(chatRoomList[i].professor.username,profUser) == 0) {
			chatRoomList[i].clientsQty = 0;
			for(j=0; j< 50; j++){
				chatRoomList[i].name[j]='\0';
			}
			chatRoomQty--;
			succesful=1;
			break;
		}
	}
	return succesful;
}


/* Find chat room by its name */
struct ChatRoom* findChatRoomByGroupName(char *name) {
	struct ChatRoom *room = NULL;
	int i;
	for (i = 0; i < MAX_CHATROOM_QTY; i++) {
		if (strcmp(chatRoomList[i].name,name) == 0) {
			room = &chatRoomList[i];
			break;
		}
	}
	return room;
}

/*
 *   Finds the Chat room by its number where the Number is its position in the array
 *   that maintains the list of chat rooms.
 *   If the number passed if smaller than 0 or bigger than MAX_CHATROOM_QTY it will return a NULL Pointer
 */
struct ChatRoom* findChatRoomByNumber(int chatRoomNumber) {
	struct ChatRoom *room = NULL;
	if (chatRoomNumber >= 0 && chatRoomNumber < MAX_CHATROOM_QTY) {
		room = &chatRoomList[chatRoomNumber];
	}
	return room;
}

/*
 *   Finds the Chat room by Professor's User Name
 *   Returns a pointer reference to the ChatRoom
 */
struct ChatRoom* findChatRoomByProfessorUser(char *profUser) {
	struct ChatRoom *room = NULL;
	int i;
	for (i = 0; i < MAX_CHATROOM_QTY; i++) {
		if (strcmp(chatRoomList[i].professor.username,profUser) == 0) {
			room = &chatRoomList[i];
			break;
		}
	}
	return room;
}

/*
 *   Adds a Client to a Chat Room.
 *   It will check if there is still room in the Chat room as it has a maximum of Clients
 *   It will not check if the user is already registered to the room
 *   If not registered in the room it will return 0 and will return 1 otherwise.
 */
int addClientToChatRoom(struct MACClient *chatClient, int chatRoomNumber) {
	int succesful = 0;
	if (chatRoomQty >= 0 && chatRoomQty <= MAX_CHATROOM_QTY) {
		if (chatRoomList[chatRoomNumber].clientsQty + 1 <  MAX_CHATROOM_CLIENTS) {
			int i;
			for(i = 0;i <= chatRoomList[chatRoomNumber].clientsQty;i++) {
				if (chatRoomList[chatRoomNumber].roomClients[i].socket == 0) {
					chatRoomList[chatRoomNumber].roomClients[i] = *chatClient;
					chatRoomList[chatRoomNumber].roomClients[i].chatRoom = chatRoomNumber;
					chatRoomList[chatRoomNumber].clientsQty++;
					succesful = 1;
					break;
				}
			}
		}
	}
	return succesful;
}



/*
 *   Finds the Chat room by a Client User Name
 *   Returns a pointer reference to the ChatRoom
 */
struct ChatRoom* findChatRoomByClientUser(char * clientUsername) {
	struct ChatRoom *room = NULL;
	int i;
	for (i = 0; i < MAX_CHATROOM_QTY; i++) {
		if(strcmp(chatRoomList[i].professor.username,clientUsername) == 0) {
			room = &chatRoomList[i];
		} else {
			int u;
			for (u = 0; u < chatRoomList[i].clientsQty; u++) {
				if (strcmp(chatRoomList[i].roomClients[u].username,clientUsername) == 0) {
					room = &chatRoomList[i];
					break;
				}
			}
		}
		if (room != NULL) {
			break;
		}
	}
	return room;
}


/*
 *   Finds the User from its Chatroom based on his User name
 *   Returns a pointer reference to the MACClient
 */
struct MACClient* findClientByUsername(char * clientUsername) {
	struct MACClient *macClient = NULL;
	int i;
	for (i = 0; i < MAX_CHATROOM_QTY; i++) {
		if(strcmp(chatRoomList[i].professor.username,clientUsername) == 0) {
			macClient = &chatRoomList[i].professor;
		} else {
			int u;
			for (u = 0; u < chatRoomList[i].clientsQty; u++) {
				if (strcmp(chatRoomList[i].roomClients[u].username,clientUsername) == 0) {
					macClient = &chatRoomList[i].roomClients[u];
					break;
				}
			}
		}
		if (macClient != NULL) {
			break;
		}
	}
	return macClient;
}

/*
 *   Finds the User from its Chatroom based on his Socket
 *   Returns a pointer reference to the MACClient
 */
struct MACClient* findClientBySocket(int socket) {
	struct MACClient *macClient = NULL;
	int i;
	for (i = 0; i < MAX_CHATROOM_QTY; i++) {
		if(chatRoomList[i].professor.socket == socket) {
			macClient = &chatRoomList[i].professor;
		} else {
			int u;
			for (u = 0; u < chatRoomList[i].clientsQty; u++) {
				if (chatRoomList[i].roomClients[u].socket == socket) {
					macClient = &chatRoomList[i].roomClients[u];
					break;
				}
			}
		}
		if (macClient != NULL) {
			break;
		}
	}
	return macClient;
}

/*
 *   Test cases for the Chat Rooms structure
 */
int serverChatRoomTest() {
	struct MACClient professor1 = {"Kobti Ziad","kobti",1,0,1};
	struct MACClient professor2 = {"Stephanos Mavromous","stephanos",14,0,1};
	struct MACClient professor3 = {"Keith Cheung","keith",15,0,1};
	struct MACClient student1 = {"Clovis Nogueira","machadoc",2,0,0};
	struct MACClient student2 = {"Yad Singh","yad",3,0,0};
	struct MACClient student3 = {"Precious Apo","precious",4,0,0};
	struct MACClient student4 = {"Student 4","std4",5,0,0};
	struct MACClient student5 = {"Student 5","std5",6,0,0};
	struct MACClient student6 = {"Student 6","std6",7,0,0};
	struct MACClient student7 = {"Student 7","std7",8,0,0};
	struct MACClient student8 = {"Student 8","std8",9,0,0};
	struct MACClient student9 = {"Student 9","std9",10,0,0};
	struct MACClient student10 = {"Student 10","std10",11,0,0};
	struct MACClient student11 = {"Student 11","std11",12,0,0};
	struct MACClient student12 = {"Student 12","std12",13,0,0};

	struct ChatRoom *chatRoom1 = createChatRoomByProfessor(&professor1, "Advanced System Programming");
	if (chatRoom1 != NULL) {
		printf("Chatroom created!\n");
		printf("Chatroom name: %s\n",chatRoom1->name);
		printf("Chatroom number: %d\n",chatRoom1->number);
		printf("Chatroom professor: %s\n",chatRoom1->professor.name);
	} else {
		printf("Chatroom NOT CREATED!\n");
	}
	printf("\n");
	struct ChatRoom *chatRoom2 = createChatRoomByProfessor(&professor2, "Advanced Database Systems");
	if (chatRoom2 != NULL) {
		printf("Chatroom created!\n");
		printf("Chatroom name: %s\n",chatRoom2->name);
		printf("Chatroom number: %d\n",chatRoom2->number);
		printf("Chatroom professor: %s\n",chatRoom2->professor.name);
	} else {
		printf("Chatroom NOT CREATED!\n");
	}
	printf("\n");
	struct ChatRoom *chatRoom3 = createChatRoomByProfessor(&professor3, "Global Finance");
	if (chatRoom3 != NULL) {
		printf("Chatroom created!\n");
		printf("Chatroom name: %s\n",chatRoom3->name);
		printf("Chatroom number: %d\n",chatRoom3->number);
		printf("Chatroom professor: %s\n",chatRoom3->professor.name);
	} else {
		printf("Chatroom NOT CREATED!\n");
	}

	printf("\n");

	if(addClientToChatRoom(&student1, chatRoom1->number)) {
		printf("User %s added to room %d successfuly!\n",student1.username, chatRoom1->number);
	} else {
		printf("User %s NOT ADDED to room %d!\n",student1.username, chatRoom1->number);
	}
	if(addClientToChatRoom(&student2, chatRoom1->number)) {
		printf("User %s added to room %d successfuly!\n",student2.username, chatRoom1->number);
	} else {
		printf("User %s NOT ADDED to room %d!\n",student2.username, chatRoom1->number);
	}
	if(addClientToChatRoom(&student3, chatRoom1->number)) {
		printf("User %s added to room %d successfuly!\n",student3.username, chatRoom1->number);
	} else {
		printf("User %s NOT ADDED to room %d!\n",student3.username, chatRoom1->number);
	}
	if(addClientToChatRoom(&student4, chatRoom1->number)) {
		printf("User %s added to room %d successfuly!\n",student4.username, chatRoom1->number);
	} else {
		printf("User %s NOT ADDED to room %d!\n",student4.username, chatRoom1->number);
	}
	printf("\n");


	if(addClientToChatRoom(&student5, chatRoom2->number)) {
		printf("User %s added to room %d successfuly!\n",student5.username, chatRoom2->number);
	} else {
		printf("User %s NOT ADDED to room %d!\n",student5.username, chatRoom2->number);
	}
	if(addClientToChatRoom(&student6, chatRoom2->number)) {
		printf("User %s added to room %d successfuly!\n",student6.username, chatRoom2->number);
	} else {
		printf("User %s NOT ADDED to room %d!\n",student6.username, chatRoom2->number);
	}
	if(addClientToChatRoom(&student7, chatRoom2->number)) {
		printf("User %s added to room %d successfuly!\n",student7.username, chatRoom2->number);
	} else {
		printf("User %s NOT ADDED to room %d!\n",student7.username, chatRoom2->number);
	}
	if(addClientToChatRoom(&student8, chatRoom2->number)) {
		printf("User %s added to room %d successfuly!\n",student8.username, chatRoom2->number);
	} else {
		printf("User %s NOT ADDED to room %d!\n",student8.username, chatRoom2->number);
	}
	printf("\n");


	if(addClientToChatRoom(&student9, chatRoom3->number)) {
		printf("User %s added to room %d successfuly!\n",student9.username, chatRoom3->number);
	} else {
		printf("User %s NOT ADDED to room %d!\n",student9.username, chatRoom3->number);
	}
	if(addClientToChatRoom(&student10, chatRoom3->number)) {
		printf("User %s added to room %d successfuly!\n",student10.username, chatRoom3->number);
	} else {
		printf("User %s NOT ADDED to room %d!\n",student10.username, chatRoom3->number);
	}
	if(addClientToChatRoom(&student11, chatRoom3->number)) {
		printf("User %s added to room %d successfuly!\n",student11.username, chatRoom3->number);
	} else {
		printf("User %s NOT ADDED to room %d!\n",student11.username, chatRoom3->number);
	}
	if(addClientToChatRoom(&student12, chatRoom3->number)) {
		printf("User %s added to room %d successfuly!\n",student12.username, chatRoom3->number);
	} else {
		printf("User %s NOT ADDED to room %d!\n",student12.username, chatRoom3->number);
	}
	printf("\n");

	struct ChatRoom *chatRoomS1 = findChatRoomByProfessorUser("kobti");
	if (chatRoomS1 != NULL) {
		printf("Chatroom found!\n");
		printf("Chatroom name: %s\n",chatRoomS1->name);
		printf("Chatroom number: %d\n",chatRoomS1->number);
		printf("Chatroom professor: %s\n",chatRoomS1->professor.name);
	} else {
		printf("Chatroom NOT FOUND!\n");
	}
	struct ChatRoom *chatRoomS2 = findChatRoomByProfessorUser("keith");
	if (chatRoomS2 != NULL) {
		printf("Chatroom found!\n");
		printf("Chatroom name: %s\n",chatRoomS2->name);
		printf("Chatroom number: %d\n",chatRoomS2->number);
		printf("Chatroom professor: %s\n",chatRoomS2->professor.name);
	} else {
		printf("Chatroom NOT FOUND!\n");
	}
	printf("\n");

	struct ChatRoom *chatRoomS3 = findChatRoomByClientUser("machadoc");
	if (chatRoomS3 != NULL) {
		printf("Chatroom found!\n");
		printf("Chatroom name: %s\n",chatRoomS3->name);
		printf("Chatroom number: %d\n",chatRoomS3->number);
		printf("Chatroom professor: %s\n",chatRoomS3->professor.name);
	} else {
		printf("Chatroom NOT FOUND!\n");
	}
	struct ChatRoom *chatRoomS4 = findChatRoomByClientUser("std7");
	if (chatRoomS4 != NULL) {
		printf("Chatroom found!\n");
		printf("Chatroom name: %s\n",chatRoomS4->name);
		printf("Chatroom number: %d\n",chatRoomS4->number);
		printf("Chatroom professor: %s\n",chatRoomS4->professor.name);
	} else {
		printf("Chatroom NOT FOUND!\n");
	}
	printf("\n");

	struct MACClient *clientS1 =findClientByUsername("machadoc");
	if (clientS1 != NULL) {
		printf("Client found!\n");
		printf("Client name: %s\n",clientS1->name);
		printf("Client username: %s\n",clientS1->username);
		printf("Client sock: %d\n",clientS1->socket);
	} else {
		printf("Client NOT FOUND!\n");
	}
	struct MACClient *clientS2 =findClientByUsername("std9");
	if (clientS2 != NULL) {
		printf("Client found!\n");
		printf("Client name: %s\n",clientS2->name);
		printf("Client username: %s\n",clientS2->username);
		printf("Client sock: %d\n",clientS2->socket);
	} else {
		printf("Client NOT FOUND!\n");
	}
}

/*
 *
 *   Main method to start the server
 */
int chatServerStart(int argc , char *argv[]) {

	//Local varibales to handle socket connections
	int socket_desc,client_sock,c,*new_sock;
	// Socket Structs for connections purposes
	struct sockaddr_in client;

	SOCKET_PORT = atoi(argv[1]);
	printf("#SERVER DEBUG# Server will listen to PORT: %d\n",SOCKET_PORT);
	printf("#SERVER DEBUG# Server will run on IP ADDRESS: %s\n",SERVER_IP);

	//Create socket to prepare to BIND and then LISTEN to new connections
	socket_desc = socket(AF_INET,SOCK_STREAM,0);
	if (socket_desc == -1) {
		printf("#SERVER DEBUG# Could not create socket, server will exit!\n");
		exit(0);
	} else {
		printf("#SERVER DEBUG# Socket was created succesfully!\n");
	}

	// Prepare the sockaddr_in structure
	server.sin_family = AF_INET;

	// server.sin_addr.s_addr = INADDR_ANY;
	server.sin_addr.s_addr = inet_addr(SERVER_IP);
	server.sin_port = htons(SOCKET_PORT);

	// Binding the Socket to then start listening to Clients connections
	if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0) {
		printf("#SERVER DEBUG# BINDING failed, server will exit!\n");
		exit(0);
	} else {
		printf("#SERVER DEBUG# BINDING done succesfully!\n");
	}

	//This method will start listenning to clientes
	// the MAX-CLIENTS parameter determines the maximum length the queue of
	// pending connections may grow to
	listen(socket_desc , MAX_CLIENTS);
	//Accept and incoming connection
	printf("#SERVER DEBUG# SERVER is now ready waiting for client connections.....\n");
	c = sizeof(struct sockaddr_in);

	while((client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c))) {
		printf("#SERVER DEBUG# Client connection accepted.....\n");
		pthread_t sniffer_thread;
		new_sock = malloc(1);
		*new_sock = client_sock;

		// Accepted Client in its new Thread, increase the quantity of connected clients
		qtyClients++;

		// Save the socket descriptor for future referenfe and communication
		sockClients[qtyClients - 1] = client_sock;
		printf("#SERVER DEBUG# Total of Clients Connected: %i\n", qtyClients);
		printf("#SERVER DEBUG# New Socket saved: %i\n", sockClients[qtyClients - 1]);
		printf("#SERVER DEBUG# Socket descriptor: %d\n", client_sock);

		if( pthread_create( &sniffer_thread,NULL,clientConnection_handler1, (void*) new_sock) < 0) {
			printf("#SERVER DEBUG# ERROR, failed to dispatch a new Thread for a client connection\n");
		} else {
			printf("#SERVER DEBUG# Thread dispatched to handle the socket client connection\n");
		}

		//Now join the thread, so that we dont terminate before the thread
		// Commented, weird behavior when using this method.
		//pthread_join(sniffer_thread , NULL);
		printServerRoomStatus();
	}
	if (client_sock < 0) {
		printf("#SERVER DEBUG# ERROR, ACCEPTING the connection to a client failed!\n");
	}

	return 0;
}

/*
 *
 *  This will handle connection for each client in each Thread
 *
 */
void *clientConnection_handler1(void *socket_desc) {

	//Get the socket descriptor
	int sock = *(int*)socket_desc;
	int read_size;                   //reading the size form read system call
	int index=0,index1=0,index2=0;   // local variables to handle loops
	char UserSelectedRoom[50];       // read the name of room selected by users
	struct MACClient *fromClient;    // handling the client informations
	char modeChar;					 // capturing the mode selected by user
	FILE *fp;						 // Capture and read user account information		  
	char *line = NULL;				 // Read data from userInfo file
	char line2[100];				 // Duplicate Read data from userInfo file
	size_t len = 0;					 // variable to read data from user
	ssize_t read1;					 // variable to read data from user
	char *token;					 // tokenize the user information
	char *token1;					 // duplicate tokenize the user information
	char userName[50];				 // username of the client
	char *unCryptPwd;				 // regular password of client
	char *password;					 // encrypted password of client
	char pwd[100];					 // encrypted password of client
	char uType[20];					 // user type information

	printf("#SERVER DEBUG# Thread for SOCKET = %d\n", sock);
	
	char client_message[2000] = "Greetings "; //reading client messages
	char client_messageDup[2000]; 			  //duplicate array for manipulations

	//formating the greeting messages to client
	strcat(client_message,"! I am your Chat server, you are connected.");

	//Send some messages to the client
	write(sock ,client_message, strlen(client_message));
	bzero(client_message,2000);


	//initialise the message buffer
	for(index=0; index< 2000; index++){
		client_message[index]='\0';
	}
	sprintf(client_message,"Enter the mode\n1. Create new User\n2. Login\n"); // ask the mode to user
	write(sock ,client_message, strlen(client_message));


	//initialise the message buffer
	for(index=0; index< 2000; index++){
		client_message[index]='\0';
	}  

	if(read_size = read(sock,client_message, 1999)<1)
	{ 
		// for(index=0; index< 2000; index++){
		// 	client_message[index]='\0';
		// }
		// sprintf(client_message,"#TERM-Fail to read data from Socket");
		// write(sock ,client_message, strlen(client_message));

		printf("#SERVER DEBUG# CTRL+D Signal Received. Terminating process\n");
		free(socket_desc);
		return 0;
	}

	//capture user selected mode
	modeChar=client_message[0];
	printf("#SERVER DEBUG#  Mode Received = <%c>\n",modeChar);

	
	//mode 1 is to create user accounts
	if(modeChar == '1'){

		//local varibale very specific to this mode of creating user accounts
		//const char *pass="Test";
		int i;  // handling loops
		char userDetails[1000]; // get the user details
		char userType; //get user type
		//generating seeds for encrypted password
		unsigned long seed[2];
		
		//creating the salt value
		char salt[] = "$1$yadwind2";

		//seedchar to be used to generate random salt value
		const char *const seedchars =
			"./0123456789ABCDEFGHIJKLMNOPQRST"
			"UVWXYZabcdefghijklmnopqrstuvwxyz";

		//initialise variables
		for(i=0; i < 1000; i++){
			userDetails[i]='\0';
		}
		//initialise variables
		for(i=0; i < 50; i++){
			userName[i]='\0';
		}
		//initialise variables
		for(index=0; index< 2000; index++){
			client_message[index]='\0';
		}

		//ask client to provide username
		sprintf(client_message,"Please provide a userName : <Please Provide single word only>\n");
		write(sock ,client_message, strlen(client_message));

		//initialise variables
		for(index=0; index< 2000; index++){
			client_message[index]='\0';
		}

		//Terminate the thread if CTRL+D (EOF) is recieved.
		if(!read(sock,client_message, 1999))
		{ 
			// for(index=0; index< 2000; index++){
			// 	client_message[index]='\0';
			// }
			// sprintf(client_message,"#TERM-Fail to read data from Socket");
			// write(sock ,client_message, strlen(client_message));

			printf("#SERVER DEBUG# CTRL+D Signal Received. Terminating process\n");
			free(socket_desc);
			return 0;
		}

		//log the input recieved
		sprintf(userName,"%s",client_message);
		printf("#SERVER DEBUG#: username:<%s>",userName);
		fflush(stdout);

		
		/* generating random seeds */
		seed[0] = time(NULL);
		seed[1] = getpid() ^ (seed[0] >> 14 & 0x30000);
		/* Turn it into printable characters from ‘seedchars’. */
		for (i = 0; i < 8; i++){
			salt[3+i] = seedchars[(seed[i/5] >> (i%5)*6) & 0x3f];
		}

		//initialise variables
		for(index=0; index< 2000; index++){
			client_message[index]='\0';
		}
		//ask for password from user
		// Send #PSWD string attached to tell the client to call getpass function to read password
		sprintf(client_message,"#PSWD-Please provide a password\n");
		write(sock ,client_message, strlen(client_message));

		//initialise variables
		for(index=0; index< 2000; index++){
			client_message[index]='\0';
		}

		//Terminate the thread if CTRL+D (EOF) is recieved.
		if(!read(sock,client_message, 1999))
		{ 
			// for(index=0; index< 2000; index++){
			// 	client_message[index]='\0';
			// }
			// sprintf(client_message,"#TERM-Fail to read data from Socket");
			// write(sock ,client_message, strlen(client_message));

			printf("#SERVER DEBUG# CTRL+D Signal Received. Terminating process\n");
			free(socket_desc);
			return 0;
		}

		//print the plain password for debugging
		unCryptPwd=client_message;
		printf("#SERVER DEBUG#: password:<%s>",unCryptPwd);
		fflush(stdout);

		//convert the password to hash string using MD5 technique
		password = crypt(unCryptPwd,salt);
		fflush(stdout);
		printf("#SERVER DEBUG#: Crypted password:<%s>",password);                
		fflush(stdout);

		//open a file add user account information
		fp=fopen("userInfo","a+");

		//reading all the existing users
		while ((read1 = getline(&line, &len, fp)) != -1) {
			token = strtok(line, "||");
			token = strtok(line, ":");
			token = strtok(NULL, ":");

			//If existing user with same name is provided, stop the process
			if(strcmp(token,userName)==0){

				for(index=0; index< 2000; index++){
					client_message[index]='\0';
				}
				sprintf(client_message,"#TERM-Username already present.\nDisconnecting\n");
				write(sock ,client_message, strlen(client_message));

				free(socket_desc);
				return 0;
			} 
			token = NULL;
		}
		fclose(fp);
		printf("FIle reading done\n");
		fflush(stdout);

		//initialise variables
		for(index=0; index< 2000; index++){
			client_message[index]='\0';
		}
		//get user type from cleint
		sprintf(client_message,"Select Type of User:\n'P' :Professsor or 'S' :Student\n");
		write(sock ,client_message, strlen(client_message));

		//Terminate the thread if CTRL+D (EOF) is recieved.
		if(!read(sock,client_message, 1999))
		{ 
			// for(index=0; index< 2000; index++){
			// 	client_message[index]='\0';
			// }
			// sprintf(client_message,"#TERM-Fail to read data from Socket");
			// write(sock ,client_message, strlen(client_message));

			printf("#SERVER DEBUG# CTRL+D Signal Received. Terminating process\n");
			free(socket_desc);
			return 0;
		}

		//print user type for debugging
		userType=client_message[0];
		printf("#SERVER DEBUG#: password:<%c>",userType);

		// If does not entered the 'P' or 'S' type, fail the process
		if(userType != 'P'  && userType != 'S'){
			for(index=0; index< 2000; index++){
				client_message[index]='\0';
			}
			//send #TERM signal to client to stop the process
			sprintf(client_message,"#TERM-Invalid User Type. Only <P> or <S> is allowed\n");
			write(sock ,client_message, strlen(client_message));
			free(socket_desc);
			return 0;
		}

		//open the file in append mode to add the new user account
		fp=fopen("userInfo","a");
		sprintf(userDetails,"Username:%s||Password:%s||UserType:%c\n",userName,password,userType);
		printf("Final = <%s>",userDetails);
		fwrite(userDetails,1,strlen(userDetails),fp);
		fclose(fp);

		//initialise variables
		for(index=0; index< 2000; index++){
			client_message[index]='\0';
		}
		//send the term signal to client to mark the process as complete 
		sprintf(client_message,"#TERM-User account created\n");
		write(sock ,client_message, strlen(client_message));

		//close the socket
		free(socket_desc);
		return 0;

	}
	else if (modeChar== '2'){
		//mode 2 is to handle the log and chatting part

		//local varibale very specific to this mode of execution
		char *result;   		// capture user inputs
		int ok;					// handle status
		int i=0;				// handle loops
		int userFound=0;	    // handle user existing functionality
		char chatRoomName[100]; // handling chat room name

		//initialise variables
		for(index=0; index< 100; index++){
			chatRoomName[index]='\0';
		}
		for(index=0; index< 2000; index++){
			client_message[index]='\0';
		}

		//ask client to provide username 
		sprintf(client_message,"Please provide a userName : <Please Provide single word only>\n");
		write(sock ,client_message, strlen(client_message));

		//initialise variables
		for(index=0; index< 2000; index++){
			client_message[index]='\0';
		}
		//Terminate the thread if CTRL+D (EOF) is recieved.
		if(!read(sock,client_message, 1999))
		{ 
			// for(index=0; index< 2000; index++){
			// 	client_message[index]='\0';
			// }
			// sprintf(client_message,"#TERM-Fail to read data from Socket");
			// write(sock ,client_message, strlen(client_message));

			printf("#SERVER DEBUG# CTRL+D Signal Received. Terminating process\n");
			free(socket_desc);
			return 0;
		}

	    //log the username for debugging
		sprintf(userName,"%s",client_message);
		printf("#SERVER DEBUG#: username Received:<%s>",userName);

		//read the user account info
		fp=fopen("userInfo","r");

		//reading the existing users and verify the the entered user is present
		while ((read1 = getline(&line, &len, fp)) != -1) {

			sprintf(line2, "%s",line);
			token1 = strtok(line, "||");
			token1 = strtok(token1, ":");
			token1 = strtok(NULL,":");
			
			// if username found capture the password and usertype information as well
			if(strcmp(token1,userName)==0){
				token=strtok(line2, "||");
				token=strtok(NULL,"||");
				sprintf(pwd,"%s",token);
				token= strtok(NULL,"||");
				token= strtok(token,":");
				token= strtok(NULL,":");
				sprintf(uType,"%s",token);
				uType[strlen(token)-1]='\0';
				printf("final UType = <%s>\n",uType);
				token=pwd;
				token= strtok(token,":");
				token= strtok(NULL,":");
				sprintf(pwd,"%s",token);
				printf("Password = <%s>",pwd);
				userFound=1;
				fflush(stdout);
				break;
			}
			token = NULL;
			token1= NULL;
		}
		fclose(fp);


		//terminate connection if username not found
		if(userFound==0){
			for(index=0; index< 2000; index++){
				client_message[index]='\0';
			}
			sprintf(client_message,"#TERM-User Not found\n");
			write(sock ,client_message, strlen(client_message));
			free(socket_desc);
			return 0;
		}


		//get password from user
		sprintf(client_message,"#PSWD-Please provide a password\n");
		write(sock ,client_message, strlen(client_message));

		//initialise variables
		for(index=0; index< 2000; index++){
			client_message[index]='\0';
		}

		//Terminate the thread if CTRL+D (EOF) is recieved.
		if(!read(sock,client_message, 1999))
		{ 
			// for(index=0; index< 2000; index++){
			// 	client_message[index]='\0';
			// }
			// sprintf(client_message,"#TERM-Fail to read data from Socket");
			// write(sock ,client_message, strlen(client_message));

			printf("#SERVER DEBUG# CTRL+D Signal Received. Terminating process\n");
			free(socket_desc);
			return 0;
		}

		//debug the client entered password
		unCryptPwd=client_message;
		printf("#SERVER DEBUG#: password:<%s>",unCryptPwd);

		//check new password again the existing hashed password
		//result should be equal to the hashed password if credentials are matched
		result = crypt(unCryptPwd, pwd);    

		//For unmatched credentails return failure
		if(strcmp (result, pwd) != 0){
			for(index=0; index< 2000; index++){
				client_message[index]='\0';
			}
			sprintf(client_message,"#TERM-Password Incorrect Access Denied\n");
			write(sock ,client_message, strlen(client_message));
			free(socket_desc);
			return 0;
		}

		//initialise variables
		for(index=0; index< 2000; index++){
			client_message[index]='\0';
		}
		sprintf(client_message,"Access Granted\n");   

		//handling the user as per the usertype
		if(uType[0] == 'P'){
			struct MACClient professor; //gather the professor credentials

			//populating the professor structure
			strcpy(professor.name,userName);
			strcpy(professor.username,userName);
			professor.socket=sock;
			professor.chatRoom=0;
			professor.isProfessor=1;

			//initialise variables
			for(index=0; index< 2000; index++){
				client_message[index]='\0';
			}

			//notify the client that they are logged in as a Professor user
			//ask them to name the group 
			sprintf(client_message,"Your are logged in as Professor. Please provide the groupName\n");
			write(sock ,client_message, strlen(client_message));

			//initialise variables
			for(index=0; index< 2000; index++){
				client_message[index]='\0';
			}

			//Terminate the thread if CTRL+D (EOF) is recieved.
			if(!read(sock,client_message, 1999))
			{ 
				// for(index=0; index< 2000; index++){
				// 	client_message[index]='\0';
				// }
				// sprintf(client_message,"#TERM-Fail to read data from Socket");
				// write(sock ,client_message, strlen(client_message));

				printf("#SERVER DEBUG# CTRL+D Signal Received. Terminating process\n");
				free(socket_desc);
				return 0;
			}

			//debug the room name entered by client
			sprintf(chatRoomName,"%s",client_message);
			printf("#SERVER DEBUG#: Room name Received : %s",chatRoomName);

			//create chat Room
			struct ChatRoom *chatRoom1 = createChatRoomByProfessor(&professor, chatRoomName);

			// if chat room creation succeed, send the  notification to client
			// else if chat room creation failed, exit the process
			if (chatRoom1 != NULL) {
				printf("#SERVER DEBUG# User added as professor and created room %d and socket %d\n",chatRoom1->number, professor.socket);
				for(index=0; index< 2000; index++){
					client_message[index]='\0';
				}
				sprintf(client_message,"Chat Room created successfuly\n");
				write(sock ,client_message, strlen(client_message));
			} else {
				printf("#SERVER DEBUG# ERROR Creating Room!\n");
				for(index=0; index< 2000; index++){
					client_message[index]='\0';
				}
				sprintf(client_message,"#TERM-ERROR Creating Room!");
				write(sock ,client_message, strlen(client_message));
				free(socket_desc);
				return 0;

			}

			//get the socket information
			fromClient = findClientBySocket(sock);
			printf("#SERVER DEBUG# MACClient name = %s\n", fromClient->name);
			printf("#SERVER DEBUG# MACClient username = %s\n", fromClient->username);
			printf("#SERVER DEBUG# MACClient Socket = %d\n", fromClient->socket);

			//initialise variables
			for(index=0; index< 2000; index++){
				client_message[index]='\0';
			}

			//Receive a message from client
			while((read_size = read(sock,client_message, 1999)) > 0) {
				//Send the message back to client
				// Broadcasts the message to all Clients registered and saved in socksClient list
				broadcastMessageToChatRoom(fromClient,client_message);
				bzero(client_message,1999);
				for(index=0; index< 2000; index++){
					client_message[index]='\0';
				}
			}
			if(read_size == 0) {
				fflush(stdout);
				if(removeChatRoom(userName))
					printf("#Server DEBUG# - Group deleted <%s>\n",chatRoomName);
			}

		}
		else{

			//handling for student profile
			struct MACClient client1 ; //structure to hold student information

			//populating the student profile
			strcpy(client1.name,userName); 
			strcpy(client1.username,userName);
			client1.socket=sock;
			client1.chatRoom=0;
			client1.isProfessor=0;

			//initialise variables
			for(index=0; index< 2000; index++){
				client_message[index]='\0';
			}
			//notify the client that they have logged in as student
			sprintf(client_message,"Your are logged in as Student.\n");
			write(sock ,client_message, strlen(client_message));

			//initialise variables
			for(index=0; index< 2000; index++){
				client_message[index]='\0';
			}
			//Ask the user to select the available groups
			sprintf(client_message,"Select the below group. Enter the exact name of the group to join\n");
			write(sock ,client_message, strlen(client_message));

			//initialise variables
			for(index=0; index< 2000; index++){
				client_message[index]='\0';
			}

			//find all the available chatrooms
			index1=1;
			for (index = 0; index < MAX_CHATROOM_QTY; index++) {
				if(chatRoomList[index].clientsQty > 0){
					sprintf(client_messageDup,"%d:%s >> By Professor :%s\n",index1,chatRoomList[index].name,chatRoomList[index].professor.name);
					strcat(client_message,client_messageDup);
					index1++;
					for(index2=0; index2< 2000; index2++){
						client_messageDup[index2]='\0';
					}
				}
			}


			printf("available list = <%s>\n",client_message);

			// if no chatroom is available, exit the process 
			if(strcmp(client_message,"")==0){
				printf("No Room available\n");
				for(index=0; index< 2000; index++){
					client_message[index]='\0';
				}
				sleep(1);
				sprintf(client_message,"#TERM-No List to select. Try After some time.\n");
				write(sock ,client_message, strlen(client_message));
				free(socket_desc);
				return 0;
			}
			else{
				//else send the chat info to client to select
				write(sock ,client_message, strlen(client_message));
			}

			//initialise variables
			for(index=0; index< 2000; index++){
				client_message[index]='\0';
			}

			//Terminate the thread if CTRL+D (EOF) is recieved.
			if(!read(sock,client_message, 1999))
			{ 
				// for(index=0; index< 2000; index++){
				// 	client_message[index]='\0';
				// }
				// sprintf(client_message,"#TERM-Error Reading from Socket.");
				// write(sock ,client_message, strlen(client_message));
				printf("#SERVER DEBUG# CTRL+D Signal Received. Terminating process\n");
				free(socket_desc);
				return 0;
			}

			//print the chat room selected by client
			sprintf(UserSelectedRoom,"%s",client_message);
			printf("User selected Room =%s\n", UserSelectedRoom);

			//find the chat room by the name selected by client
			struct ChatRoom *chatRoomS1 = findChatRoomByGroupName(UserSelectedRoom);

			//If room is not found, exit the proccess and ask the child to terminate
			if(chatRoomS1 == NULL){

				for(index=0; index< 2000; index++){
					client_message[index]='\0';
				}
				sprintf(client_message,"#TERM-Ivalid Group. Terminating!!!\n");
				write(sock ,client_message, strlen(client_message));

				free(socket_desc);
				return 0;
			}

			//Add the client to the room
			if(addClientToChatRoom(&client1,chatRoomS1->number)) {
				printf("#SERVER DEBUG# User added as client to room %d and socket %d\n",chatRoomS1->number, client1.socket);
				for(index=0; index< 2000; index++){
					client_message[index]='\0';
				}
				sprintf(client_message,"Chat Room %s Joined !!!\n",UserSelectedRoom);
				write(sock ,client_message, strlen(client_message));
			} else {

				//else exit and askk child to terminate as well
				printf("#SERVER DEBUG# User ERROR\n");

				for(index=0; index< 2000; index++){
					client_message[index]='\0';
				}
				sprintf(client_message,"#TERM- Not able to add to room. Terminating!!!\n");
				write(sock ,client_message, strlen(client_message));

				free(socket_desc);
				return 0;
			}

			//read socket information
			fromClient = findClientBySocket(sock);
			printf("#SERVER DEBUG# MACClient name = %s\n", fromClient->name);
			printf("#SERVER DEBUG# MACClient username = %s\n", fromClient->username);
			printf("#SERVER DEBUG# MACClient Socket = %d\n", fromClient->socket);

			//initialise variables
			for(index=0; index< 2000; index++){
				client_message[index]='\0';
			}
			//Receive a message from client
			while((read_size = read(sock,client_message, 1999)) > 0) {
				int i;
				//Send the message back to client
				// Broadcasts the message to all Clients registered and saved in socksClient list
				broadcastMessageToChatRoom(fromClient,client_message);
				bzero(client_message,1999);
				for(index=0; index< 2000; index++){
					client_message[index]='\0';
				}
			}
		}
	}
	else{

		//invalid mode recieved
		for(index=0; index< 2000; index++){
			client_message[index]='\0';
		}
		sprintf(client_message,"#TERM-Invalid option Selected.\nTerminating connection\n");
		write(sock ,client_message, strlen(client_message));
		free(socket_desc);
		return 0;
	}

	//CTRL+D or (EOF) signal is recieved
	//close the socket
	if(read_size == 0) {
		printf("#SERVER DEBUG# MACClient username = <%s> disconnected!\n", userName);
		fflush(stdout);
	} else if(read_size == -1) {
		perror("recv failed");
	}
	//Free the socket pointer
	free(socket_desc);

	return 0;
}


/*
 *  This method is responsible for Cleaning up the Client Disconnected
 */
void cleanOutDisconectedClient(struct MACClient *macClient) {
	qtyClients--;
	struct ChatRoom* userChatRoom = findChatRoomByNumber(macClient->chatRoom);
	if (!macClient->isProfessor) {
		userChatRoom->clientsQty--;
		strcpy(macClient->name,"");
		strcpy(macClient->username,"");
		macClient->socket = 0;
		macClient->chatRoom = 0;
		macClient->isProfessor = 0;
	} else {
		// In this case it is a professor, should delete the room????
	}
}

/*
 *  This method is responsible for broadcasting messages to all the Chat Room clients
 */
int broadcastMessageToChatRoom(struct MACClient *clientFrom, char *msg) {
	printf("#SERVER BROADCAST# Message Broadcast method\n");
	printf("#SERVER BROADCAST# Message From: %s\n", clientFrom->name);
	struct ChatRoom* userChatRoom = findChatRoomByNumber(clientFrom->chatRoom);

	if (userChatRoom != NULL) {
		printf("#SERVER BROADCAST# Room name : %s\n", userChatRoom->name);
	} else {
		printf("#SERVER BROADCAST# ChatRoom NOT FOUND!\n");
	}

	int userIndex;
	for (userIndex = 0; userIndex < userChatRoom->clientsQty; userIndex++) {
		int socket = userChatRoom->roomClients[userIndex].socket;
		printf("#SERVER BROADCAST# Broadcasted Message to Username: %s\n",userChatRoom->roomClients[userIndex].username);
		if (socket != clientFrom->socket) {
			char finalMsg[2000] = "# From: ";
			strcat(finalMsg, clientFrom->name);
			strcat(finalMsg, "\nMessage: ");
			strcat(finalMsg, msg);
			strcat(finalMsg, "\n");
			// Write message here
			write(socket,finalMsg, strlen(finalMsg));
		}
	}
}

/*
 *  This method is can be used to DEBUG Purposes and it prints the actual status
 *  of the Chat Room Structure
 */
void printServerRoomStatus() {
	int chatIndex, clientIndex;
	printf("######################################################################\n");
	printf("#SERVER DEBUG# Room total: %d\n",chatRoomQty);
	for(chatIndex = 0; chatIndex < chatRoomQty; chatIndex++) {
		printf("######################################################################\n");
		printf("#SERVER DEBUG# Room Index: %d\n",chatRoomList[chatIndex].number);
		printf("#SERVER DEBUG# Room Name: %s\n",chatRoomList[chatIndex].name);
		printf("#SERVER DEBUG# Room Qty Clients: %d\n",chatRoomList[chatIndex].clientsQty);
		printf("#SERVER DEBUG# Room Professor Name: %s\n",chatRoomList[chatIndex].professor.name);
		printf("#SERVER DEBUG# Room Professor User Name: %s\n",chatRoomList[chatIndex].professor.username);
		printf("#SERVER DEBUG# Room Professor Socket: %d\n",chatRoomList[chatIndex].professor.socket);
		printf("#SERVER DEBUG# Room Is professor Client: %d\n",chatRoomList[chatIndex].professor.isProfessor);
		printf("#SERVER DEBUG# Room Professor Room: %d\n",chatRoomList[chatIndex].professor.chatRoom);
		for (clientIndex = 0;clientIndex < chatRoomList[chatIndex].clientsQty; clientIndex++) {
			printf("\t#SERVER DEBUG# Room Client Name: %s\n",chatRoomList[chatIndex].roomClients[clientIndex].name);
			printf("\t#SERVER DEBUG# Room Client User Name: %s\n",chatRoomList[chatIndex].roomClients[clientIndex].username);
			printf("\t#SERVER DEBUG# Room Client Socket: %d\n",chatRoomList[chatIndex].roomClients[clientIndex].socket);
			printf("\t#SERVER DEBUG# Room Is professor Client: %d\n",chatRoomList[chatIndex].roomClients[clientIndex].isProfessor);
			printf("\t#SERVER DEBUG# Room Client Room: %d\n",chatRoomList[chatIndex].roomClients[clientIndex].chatRoom);
		}
		printf("#######################################################################\n");
	}

}

/*
 *  This main method to start the server
 */
int main(int argc , char *argv[]) {
	if (argc != 2) {
		printf("Provide the SOCKET PORT to start the server\n");
		printf("Example: ChatServer 8888\n");
		exit(0);
	}
	// Start the Server here
	chatServerStart(argc, argv);
}

