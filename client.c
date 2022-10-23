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

void sigintHandler(int signum);

int readServerInfo(char* info_file_path);
int createClientPipe();
int connectServer(int server_pipe_fd, Client client, int timeout);
void deconnectServer();

void handleMessage(Message);
void readPlayerList(int, Client*);
Client chosePlayer(int, Client*);

void trimCarriageReturn(char*);

// Global variable for access in sigintHandler
Client me;
int server_pipe_fd;

int main ()
{
	// Register signal handler
	signal(SIGINT, sigintHandler);

	// Get client identity
	me.is_spy = 0;
	me.pipe_fd = createClientPipe();
	me.PID = getpid();
	printf("Pseudo : ");
	fgets(me.pseudo, PSEUDO_MAX_SIZE, stdin);
	trimCarriageReturn(me.pseudo);

	// Connect to server
	server_pipe_fd = readServerInfo(SERVER_INFO_FILE_PATH);
 	connectServer(server_pipe_fd, me, 0);

	Message message_buffer;
	int byte_read = 0;
	while (1)
   	{
		byte_read = read(me.pipe_fd, &message_buffer, sizeof(Message));
		if (!byte_read || byte_read == EOF)
			continue;

		handleMessage(message_buffer);
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

void handleMessage(Message message)
{
	switch (message.command)
	{
		case VOTE:
			Client player_list[SERVER_MAX_CLIENTS];
			readPlayerList(message.data, player_list);
			Client voted = chosePlayer(message.data, player_list);
			Message vote_message = {
				me,
				voted,
				VOTE
			};
			write(server_pipe_fd, &vote_message, sizeof(Message));
			break;
		
		case ASK:
			printf("%s\n", message.message);
			char word[STRING_MAX_SIZE] = {0};
			scanf("%s", word);
			Message answer_message = {
				me,
				me,
				MESSAGE
			};
			strcpy(answer_message.message, word);
			write(server_pipe_fd, &answer_message, sizeof(Message));
			break;

		case ASK_TO:

			readPlayerList(message.data, player_list);
			//Client player2_list[SERVER_MAX_CLIENTS];
			//printf("je suis la qd meme\n");
			//readPlayerList(2, player2_list);
			break;

	}
}

void readPlayerList(int n, Client* player_list)
{
	Message message_buffer;
	for (int i = 0; i < n; i++)
	{
		read(me.pipe_fd, &message_buffer, sizeof(Message));
		player_list[i] = message_buffer.from;
	}
}

Client chosePlayer(int n, Client* player_list)
{
	int choice = 0;
	for (int i = 0; i < n; i++)
	{
		printf("%d. %s\n", i+1, player_list[i].pseudo);
	}
	scanf("%d", &choice);

	return player_list[choice-1];
}

void sigintHandler(int signum)
{
	deconnectServer();
	exit(0);
}

void trimCarriageReturn(char* str)
{
	str[strlen(str)-1]=0;
}
