#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>

#define NAME_PIPE_CLIENT 8


int createClientPipe();
//	create a named pipe for communicate with the server and return its file descriptor.
//	Name of the pipe is PID.

int main (int argc, char *agv[]){

	printf("CREAT = %d\n",createClientPipe());




	return 0;
}



int createClientPipe(){

	char clientPipe[NAME_PIPE_CLIENT];
	
	sprintf(clientPipe,"%d",getpid());

       if(mkfifo(clientPipe, 0666)== -1){
       
       	return -1;
       }

       return open(clientPipe,O_RDWR);
}
