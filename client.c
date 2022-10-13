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

int connectServer(int server_pipe_fd, int client_pipe_fd, int timeout);
//	Write PID to base_server_pipe_fd.
//
//	Return 1 if server write 1 in process named pipe.
//	Return 0 if no write before timeout.

int main (int argc, char *agv[]){

	int client_pipe_fd = createClientPipe();
	printf("CREAT = %d\n",client_pipe_fd);


int desc=0;
  char* info_file_path ="servinfo";
    desc = readServerInfo(info_file_path);
    
    
   connectServer(desc, client_pipe_fd, 0);
   char buf[64] = {0};
   while (1)
   {
   	scanf("%s", buf);
	write(client_pipe_fd, buf, 7);
   }
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

int connectServer(int server_pipe_fd, int client_pipe_fd, int timeout){

	char message[NAME_PIPE_CLIENT];

	sprintf(message,"%d",getpid());

	write(server_pipe_fd,message,8*sizeof(char));

}

