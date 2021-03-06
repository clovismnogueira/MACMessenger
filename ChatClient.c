/*
 *
 *   This is the Client for the Chat application running over sockets
 *
 */

//header files
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>

//global varibale
int sock;
char *SERVER_IP = "137.207.82.53"; // Server IP Address
int SOCKET_PORT = 0;
int chatClientStart(int argc , char *argv[]);
void *connection_READER_handler(void *);
void *connection_WRITER_handler(void *);
int passwdToken=0;

/*
 *    Method to start the Client Chat
 */
int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("Provide the SOCKET PORT to start the server\n");
		printf("Example: ChatClient 8888\n");
		exit(0);
	}
	// Start the Server here
	chatClientStart(argc, argv);

}

/*
 *    Method to start the Client Chat
 */
int chatClientStart(int argc , char *argv[]) {
	struct sockaddr_in server;
	char message[1000] , server_reply[2000];

	SOCKET_PORT = atoi(argv[1]);

	//Create socket
	sock = socket(AF_INET , SOCK_STREAM , 0);
	if (sock == -1) {
		printf("Could not create socket");
	}
	printf("Socket created");
	//server.sin_addr.s_addr = inet_addr("192.168.111.128");
	server.sin_addr.s_addr = inet_addr("137.207.82.53");
	server.sin_family = AF_INET;
	server.sin_port = htons(SOCKET_PORT);

	//Connect to remote server
	if (connect(sock,(struct sockaddr *)&server,sizeof(server)) < 0) {
		perror("connect failed. Error");
		return 1;
	}

	printf("Connected\n");

	// Starts the Thread to Listen messages from ChatRoom
	pthread_t sniffer_thread_read;
	if( pthread_create(&sniffer_thread_read,NULL,connection_READER_handler, NULL) < 0) {
		perror("could not create thread to Listen to Server Messages");
		return 1;
	}
	// Starts the Thread to Write messages to the ChatRoom
	pthread_t sniffer_thread_write;
	if( pthread_create(&sniffer_thread_write,NULL,connection_WRITER_handler, NULL) < 0) {
		perror("could not create thread to Send Messages to the Server");
		return 1;
	}

	while (1) {
		// do nothing just keep process alive
	}
}


/*
 * This will handle connection READING FROM SOCKET for each client
 * */
void *connection_READER_handler(void* x) {
	int read_size;
	// Limits to read client message to 1999 Char
	char server_message[2000];
	bzero(server_message,1999);
	char *token;

	while((read_size = read(sock,server_message, 1999)) > 0 ) {

		token=strtok(server_message,"-");
		//closing the process in recieving the TERM string
		if(strcmp(token,"#TERM")==0){
			token=strtok(NULL,"-");
			printf("\nMessage recieved: %s\n",token);
			printf("Terminating the process\n");
			exit(0);
		}
		else if(strcmp(token,"#PSWD")==0){
			//ask the write thread to read the password special format
			token=strtok(NULL,"-");
			printf("\nMessage recieved: %s\n",token);
			passwdToken=1;
			continue;
		}
		printf("\nMessage Received: %s", server_message);
		bzero(server_message,256);
		printf("\nEnter message: ");
	}
	printf("Server Terminated.\n");
	printf("Killing the current process\n");
	close(sock);
	exit(1);
}

/*
 * This will handle connection WRITTING TO SOCKET for each client
 *
 */
void *connection_WRITER_handler(void* x) {
	int read_size;
	// Limits to read client message to 2000 Char
	char client_message[2000];
	bzero(client_message,1999);
	char *password;

	while(1) {
		sleep(1);
		if(passwdToken==1){
			//use getpass to read password
			passwdToken=0;
			password=getpass("Password:");
			sprintf(client_message,"%s",password);

		}
		else{
			scanf("%[^\n]%*c", client_message);
		}
		if(send(sock,client_message,strlen(client_message),0) < 0) {
			printf("Send failed");
		}
		bzero(client_message,1999);
	}
}

