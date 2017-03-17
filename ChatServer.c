/*
*
*   This is the Servet for the Chat application running over sockets
*
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
int const MAX_CLIENTS = 200;
//the thread heandler function to handle client connections
void *clientConnection_handler(void *);
int sockClients[200];
int qtyClients = 0;
int SOCKET_PORT = 0;

/*
*
*   Main method to start the server
*/
int main(int argc , char *argv[]) {
    int socket_desc,client_sock,c,*new_sock;
    // Socket Structs for connections purposes
    struct sockaddr_in server;
    struct sockaddr_in client;
    SOCKET_PORT = 9920;

    // if (argc != 2) {
    //   printf("Provide the SOCKET PORT to start the server\n");
    //   printf("Example: ChatServer 10000\n");
    //   exit(0);
    // }

    printf("#SERVER DEBUG# Server will listen to PORT: %d\n",SOCKET_PORT);


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
    server.sin_addr.s_addr = INADDR_ANY;
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
