/*
*
*   This is the Servet for the Chat application running over sockets
*
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

// This constant defines the maximum of clients to be connected to this Server
int static const MAX_CLIENTS = 200;
int static const MAX_NAME_SIZE = 50;
int static const MAX_USERNAME_SIZE = 50;
int static const MAX_CHATROOM_CLIENTS = 50;
int static const MAX_CHATROOM_QTY = 10;

//the thread heandler function to handle client connections
void *clientConnection_handler(void *);
int sockClients[200];
int qtyClients = 0;
int SOCKET_PORT = 0;
char *SERVER_IP = "192.168.111.128";

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

// List of Chatroom
struct ChatRoom chatRoomList[10];
int chatRoomQty = 0;
struct sockaddr_in server;


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
      //  printf("UserName struct = %s\n",chatRoomList[i].professor.username);
      //  printf("UserName passed = %s\n",profUser);
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
      int clientIndex = chatRoomList[chatRoomNumber].clientsQty;
      chatRoomList[chatRoomNumber].roomClients[clientIndex] = *chatClient;
      chatRoomList[chatRoomNumber].clientsQty++;
      succesful = 1;
    }
  }
  return succesful;
}

/*
*   Finds the Chat room by Professor's User Name
*   Returns a pointer reference to the ChatRoom
*/
struct ChatRoom* createChatRoomByProfessor(struct MACClient *chatProfessor, char *roomName) {
  struct ChatRoom *room = NULL;
  int i;
  for (i = 0; i < MAX_CHATROOM_QTY; i++) {
     if (chatRoomList[i].clientsQty == 0) {
       chatRoomList[i].professor = *chatProfessor;
       chatRoomList[i].clientsQty = 1;
       chatRoomList[i].number = i+1;
       strcpy(chatRoomList[i].name,roomName);
       room = &chatRoomList[i];
       break;
     }
  }
  return room;
}

/*
*   Finds the Chat room by a Client User Name
*   Returns a pointer reference to the ChatRoom
*/
struct ChatRoom* findChatRoomByClientUser(char * clientUser) {
  struct ChatRoom *room = NULL;
  int i;
  for (i = 0; i < MAX_CHATROOM_QTY; i++) {
    int u;
    // printf("Chatroom name = %s\n",chatRoomList[i].name);
    for (u = 0; u < chatRoomList[i].clientsQty; u++) {
      // printf("User name %i = %s\n",u, chatRoomList[i].roomClients[u].name);
      // printf("Searched User name = %s\n",clientUser);
      if (strcmp(chatRoomList[i].roomClients[u].username,clientUser) == 0) {
        room = &chatRoomList[i];
        break;
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
    int u;
    // printf("Chatroom name = %s\n",chatRoomList[i].name);
    for (u = 0; u < chatRoomList[i].clientsQty; u++) {
      // printf("User name %i = %s\n",u, chatRoomList[i].roomClients[u].name);
      // printf("Searched User name = %s\n",clientUser);
      if (strcmp(chatRoomList[i].roomClients[u].username,clientUsername) == 0) {
        macClient = &chatRoomList[i].roomClients[u];
        break;
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

        if( pthread_create( &sniffer_thread,NULL,clientConnection_handler, (void*) new_sock) < 0) {
            printf("#SERVER DEBUG# ERROR, failed to dispatch a new Thread for a client connection\n");
        } else {
            printf("#SERVER DEBUG# Thread dispatched to handle the socket client connection\n");
        }

        //Now join the thread, so that we dont terminate before the thread
        // Commented, weird behavior when using this method.
        //pthread_join(sniffer_thread , NULL);
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
void *clientConnection_handler(void *socket_desc) {
    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    int read_size;
    char *message;

    //Send some messages to the client
    message = "Greetings! I am your Chat server, you are connected!\n";
    write(sock , message , strlen(message));
    printf("#SERVER REPLY# %s\n", message);
    // Limits to read client message to 255 Char
    char client_message[256];
    bzero(client_message,256);
    //Receive a message from client
    while((read_size = read(sock,client_message, 255)) > 0) {
        int i;
        //Send the message back to client
        // Broadcasts the message to all Clients registered and saved in socksClient list
        for (i = 0; i < qtyClients; i++) {
            printf("#SERVER BROADCAST# Message %s to socket => %i\n",client_message, sockClients[qtyClients - 1]);
            if (sockClients[i] == sock) {
              printf("#SERVER BROADCAST SKIP# Message skipped to socket => %i\n", sockClients[qtyClients - 1]);
            } else {
              write(sockClients[i],client_message, strlen(client_message));
            }
        }
        bzero(client_message,256);
    }

    if(read_size == 0) {
        puts("Client disconnected");
        fflush(stdout);
    } else if(read_size == -1) {
        perror("recv failed");
    }

    //Free the socket pointer
    free(socket_desc);

    return 0;
}


int main(int argc , char *argv[]) {

  if (argc != 2) {
    printf("Provide the SOCKET PORT to start the server\n");
    printf("Example: ChatServer 8888\n");
    exit(0);
  }

  // Start the Server here
  //chatServerStart(argc, argv);
}
