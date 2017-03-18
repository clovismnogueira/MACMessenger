/*
*
*   This is the Server for the Chat application running over sockets
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
char *SERVER_IP = "192.168.111.128";
int SOCKET_PORT = 0;

// Method signatures
void printServerRoomStatus();
void cleanOutDisconectedClient(struct MACClient *macClient);
struct MACClient* findClientBySocket(int socket);
int broadcastMessageToChatRoom(struct MACClient *clientFrom, char *msg);
//the thread heandler function to handle client connections
void *clientConnection_handler(void *);

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
*   Finds the Chat room by Professor's User Name
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

        if( pthread_create( &sniffer_thread,NULL,clientConnection_handler, (void*) new_sock) < 0) {
            printf("#SERVER DEBUG# ERROR, failed to dispatch a new Thread for a client connection\n");
        } else {
            printf("#SERVER DEBUG# Thread dispatched to handle the socket client connection\n");
            // Code for testing purposes only!!!!
            // needs to be removed after integrated
            switch (qtyClients) {
              case 1 :
              {
                struct MACClient professor1 = {"Kobti Ziad","kobti",client_sock,0,1};
                struct ChatRoom *chatRoom1 = createChatRoomByProfessor(&professor1, "Advanced System Programming");
                if (chatRoom1 != NULL) {
                  printf("#SERVER DEBUG# User 1 added as professor and created room %d and socket %d\n",chatRoom1->number, professor1.socket);
                } else {
                  printf("#SERVER DEBUG# User 1 ERROR!\n");
                }
                break;
              }
              case 2:
              {
                struct MACClient client1 = {"Clovis Nogueira","machadoc",client_sock,0,0};
                struct ChatRoom *chatRoomS1 = findChatRoomByProfessorUser("kobti");
                if(addClientToChatRoom(&client1,chatRoomS1->number)) {
                  printf("#SERVER DEBUG# User 2 added as client to room %d and socket %d\n",chatRoomS1->number, client1.socket);
                } else {
                  printf("#SERVER DEBUG# User 2 ERROR\n");
                }
                break;
              }
              case 3:
              {
                struct MACClient client1 = {"Yadwinder Singh","yad",client_sock,0,0};
                struct ChatRoom *chatRoomS1 = findChatRoomByProfessorUser("kobti");
                if(addClientToChatRoom(&client1,chatRoomS1->number)) {
                  printf("#SERVER DEBUG# User 3 added as client to room %d and socket %d\n",chatRoomS1->number, client1.socket);
                } else {
                  printf("#SERVER DEBUG# User 3 ERROR\n");
                }
                break;
              }
              case 4 :
              {
                struct MACClient professor1 = {"Stephanos Something","stephanos",client_sock,0,1};
                struct ChatRoom *chatRoom1 = createChatRoomByProfessor(&professor1, "Advanced Database Systems");
                if (chatRoom1 != NULL) {
                  printf("#SERVER DEBUG# User 4 added as professor and created room %d and socket %d\n",chatRoom1->number, professor1.socket);
                } else {
                  printf("#SERVER DEBUG# User 4 ERROR!\n");
                }
                break;
              }
              case 5:
              {
                struct MACClient client1 = {"Edne Nogueira","ednecris",client_sock,0,1};
                struct ChatRoom *chatRoomS1 = findChatRoomByProfessorUser("stephanos");
                if(addClientToChatRoom(&client1,chatRoomS1->number)) {
                  printf("#SERVER DEBUG# User 5 added as client to room %d and socket %d\n",chatRoomS1->number, client1.socket);
                } else {
                  printf("#SERVER DEBUG# User 5 ERROR\n");
                }
                break;
              }
              case 6:
              {
                struct MACClient client1 = {"Precious Akporah","precious",client_sock,0,1};
                struct ChatRoom *chatRoomS1 = findChatRoomByProfessorUser("stephanos");
                if(addClientToChatRoom(&client1,chatRoomS1->number)) {
                  printf("#SERVER DEBUG# User 6 added as client to room %d and socket %d\n",chatRoomS1->number, client1.socket);
                } else {
                  printf("#SERVER DEBUG# User 6 ERROR\n");
                }
                break;
              }
            }
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
void *clientConnection_handler(void *socket_desc) {
    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    int read_size;
    printf("#SERVER DEBUG# Thread for SOCKET = %d\n", sock);
    struct MACClient *fromClient = findClientBySocket(sock);
    printf("#SERVER DEBUG# MACClient name = %s\n", fromClient->name);
    printf("#SERVER DEBUG# MACClient username = %s\n", fromClient->username);
    printf("#SERVER DEBUG# MACClient Socket = %d\n", fromClient->socket);

    char client_message[2000] = "Greetings ";
    strcat(client_message,fromClient->name);
    strcat(client_message,"! I am your Chat server, you are connected.");
    //Send some messages to the client
    write(sock ,client_message, strlen(client_message));
    bzero(client_message,2000);

    //Receive a message from client
    while((read_size = read(sock,client_message, 1999)) > 0) {
        int i;
        //Send the message back to client
        // Broadcasts the message to all Clients registered and saved in socksClient list
        broadcastMessageToChatRoom(fromClient,client_message);
        bzero(client_message,1999);
    }

    if(read_size == 0) {
        printf("#SERVER DEBUG# MACClient username = %s disconnected!\n", fromClient->username);
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
