/*
    C ECHO client example using sockets
*/
#include <stdio.h> //printf
#include <string.h>    //strlen
#include <sys/socket.h>    //socket
#include <arpa/inet.h> //inet_addr
#include <pthread.h> //for threading , link with lpthread
#include <unistd.h>    //write


void *connection_READER_handler(void *);
void *connection_WRITER_handler(void *);
int sock;

int main(int argc , char *argv[]) {

    struct sockaddr_in server;
    char message[1000] , server_reply[2000];

    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");

    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(9920);

    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        return 1;
    }

    puts("Connected\n");

    pthread_t sniffer_thread_read;

    if( pthread_create( &sniffer_thread_read , NULL ,  connection_READER_handler , NULL) < 0)
    {
        perror("could not create thread to Listen to Server Messages");
        return 1;
    }

    pthread_t sniffer_thread_write;

    if( pthread_create( &sniffer_thread_write , NULL ,  connection_WRITER_handler , NULL) < 0)
    {
        perror("could not create thread to Send Messages to the Server");
        return 1;
    }

    while (1) {
      // do nothing just keep process alive
    }

}


/*
 * This will handle connection for each client
 * */
void *connection_READER_handler(void* x)
{
    int read_size;

    // Limits to read client message to 255 Char
    char server_message[256];
    bzero(server_message,256);

    while((read_size = read(sock,server_message, 255)) > 0 )
    {
        printf("\nMessage Received: %s", server_message);
        bzero(server_message,256);
        printf("\nEnter message: ");
    }

}

/*
 * This will handle connection for each client
 * */
void *connection_WRITER_handler(void* x)
{
    int read_size;
    // Limits to read client message to 255 Char
    char client_message[256];
    bzero(client_message,256);
    char message[1000];

    // int row,col;
    // initscr();
    // getmaxyx(stdscr,row,col);
    // mvprintw(row/2,(col-str))


    while(1) {
        printf("\nEnter message : ");
        scanf ("%[^\n]%*c", message);
        // printf("#CLIENT DEBUG# Client Message=> %s\n" , message);

        if( send(sock , message , strlen(message) , 0) < 0)
        {
            puts("Send failed");
        }
    }
}
