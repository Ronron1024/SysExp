#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>

#define NAME_PIPE_CLIENT 8

#define MAX 15


int readServerInfo(char* info_file_path);


int createClientPipe();
//	create a named pipe for communicate with the server and return its file descriptor.
//	Name of the pipe is PID.

int main (int argc, char *agv[]){

	printf("CREAT = %d\n",createClientPipe());

int desc=0;
  char* info_file_path ="servinfo";
    desc = readServerInfo(info_file_path);
    
    
    write(desc, "coucou ",7) ;
    close(desc);





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



int readServerInfo(char* info_file_path){
	
	int dp=0;
    int desc=0;
	FILE* file = fopen(info_file_path, "r");
	char buf[MAX];
    fgets(buf, MAX, file);
    
    
    printf("pipe is: %s\n", buf);
	
    dp=open(buf,O_WRONLY);
    close(desc);
	return dp;
}
