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
#define SERVER_INFO_FILE_PATH "servinfo"


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

	printf("CREATED CLIENT PIPE = %d\n",client_pipe_fd);


	int server_pipe_fd = readServerInfo(SERVER_INFO_FILE_PATH);
    
    
 	connectServer(server_pipe_fd, client_pipe_fd, 0);
	char buf[64] = {0};
	int byte_read = 0;
   	while (1)
   	{
   		fgets(buf, 64, stdin);
		write(client_pipe_fd, buf, strlen(buf)*sizeof(char));
		usleep(100);
		byte_read = read(client_pipe_fd, buf, 64);
		buf[byte_read] = 0;
		printf("ANSWER : %s", buf);
	}
	close(server_pipe_fd);
	close(client_pipe_fd);
	// MUST UNLINK CLIENT PIPE
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
	
	int server_pipe_fd=0;
	FILE* file = fopen(info_file_path, "r");
	char buf[MAX];
    	fgets(buf, MAX, file);
    
    
    	printf("server pipe is: %s\n", buf);
	
    	server_pipe_fd=open(buf,O_RDWR);
    	fclose(file);
	return server_pipe_fd;
}

int connectServer(int server_pipe_fd, int client_pipe_fd, int timeout){

	char message[NAME_PIPE_CLIENT];

	sprintf(message,"%d",getpid());

	write(server_pipe_fd,message,strlen(message)*sizeof(char));
}

