#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>

#include "server_config.h"
#include "structures.h"

int readServerInfo(char* info_file_path);
int createClientPipe();
int connectServer(int server_pipe_fd, Client client, int timeout);
void deconnectServer();

void sigintHandler(int signum);

void trimCarriageReturn(char*);

// Global variable for access in sigintHandler
Client me;
int server_pipe_fd;
pid_t reader = 0;

int main ()
{
	// Register signal handler
	signal(SIGINT, sigintHandler);

	// Get client identity
	me.is_spy = -1;
	me.pipe_fd = createClientPipe();
	me.PID = getpid();
	printf("Pseudo : ");
	fgets(me.pseudo, STRING_MAX_SIZE, stdin);
	trimCarriageReturn(me.pseudo);

	// Connect to server
	server_pipe_fd = readServerInfo(SERVER_INFO_FILE_PATH);
 	connectServer(server_pipe_fd, me, 0);

	Message message_buffer;
	
	// Read message from other clients
	reader = fork();
	if (!reader)
	{
		// Unregister SIGINT for prevent double deconnection
		signal(SIGINT, NULL);

		while (1)
		{
			read(me.pipe_fd, &message_buffer, sizeof(Message));
			printf("\n[%s] %s\n", message_buffer.client.pseudo, message_buffer.message);	
		}
	}

	while (1)
   	{
		message_buffer.client = me;
		message_buffer.command = MESSAGE;
   		fgets(message_buffer.message, STRING_MAX_SIZE, stdin);

		write(server_pipe_fd, &message_buffer, sizeof(Message));
	}

	return 0;
}

int createClientPipe()
{
	char clientPipe[STRING_MAX_SIZE];
	sprintf(clientPipe,"%d",getpid());

	if(mkfifo(clientPipe, 0666) == -1)
		return -1;

	return open(clientPipe,O_RDWR);
}

int readServerInfo(char* info_file_path)
{	
	FILE* info_file = fopen(info_file_path, "r");
	
	if (!info_file)
	{
		fprintf(stderr, "Error while opening %s\n", info_file_path);
		return -1;
	}

	// Read server info file
	char buf[STRING_MAX_SIZE];
    	fgets(buf, STRING_MAX_SIZE, info_file);
    	fclose(info_file);
	
	return open(buf,O_RDWR);
}

int connectServer(int server_pipe_fd, Client client, int timeout)
{
	Message message = {
		client,
		CONNECTION,
		"This is the connection message"
	};
	write(server_pipe_fd, &message, sizeof(Message));

	return 1;
}

void deconnectServer()
{
	Message message = {
		me,
		DECONNECTION,
		"This is the deconnection message"
	};
	write(server_pipe_fd, &message, sizeof(Message));

	close(server_pipe_fd);
	close(me.pipe_fd);
	
	char pipe_name[STRING_MAX_SIZE] = {0};
	sprintf(pipe_name, "%d", me.PID);
	unlink(pipe_name);
}

void sigintHandler(int signum)
{
	deconnectServer();
	kill(reader, SIGINT);
	wait(NULL);
	exit(0);
}

void trimCarriageReturn(char* str)
{
	str[strlen(str)-1]=0;
}
