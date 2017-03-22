#include <stdio.h>
#include <stdlib.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>



#define CHILD_CREAT 401
#define CWD_ARRAY_SZ 1024

void myhandler(){
	int status;
	int i = wait(&status);
	printf("child terminated\n");
}

int main(int argc, char *argv[]){

	int i,index;
	int pid;
	int activeChild[CHILD_CREAT];
	int count=0;
	char t3[]="ChatClient.exe";
	char t2[]="10080";
	char currWorkDir[CWD_ARRAY_SZ];
	for(i=0; i<CWD_ARRAY_SZ;i++){
		currWorkDir[i]= '\0';
	}


	signal(SIGCHLD, myhandler);
	/*=======================================================*/
	/*     Get current working directory information         */
	/*=======================================================*/
	if(getcwd(currWorkDir,sizeof(currWorkDir)) == NULL){
		printf("Error in finding the current working directory.");
		printf("Aborting the process.\n");
	}

	sprintf(currWorkDir + strlen(currWorkDir),"/");
	sprintf(currWorkDir + strlen(currWorkDir),t3);


	for(i=0; i<CHILD_CREAT; i++){

		pid=fork();
		if(!pid){
			printf("Child forking\n");

			if(execl (currWorkDir,t3,t2,NULL) == -1){
				printf("Child Execution failed\n");
			}
			else{
				printf("Child created succesfully\n");
			}
		}
		activeChild[count] = pid;
		count++;
	}
	
	sleep(5);
	printf("Killing Child\n");
	for(i=0; i < count ; i++){
		printf("Killing %d Client to Chat Server  with PID = %d\n",i+1,activeChild[i]);
		kill(activeChild[i],SIGTERM);
	}

}
