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
	printf("Connected to server\n");

	// Start client tchat
	pid_t tchat_pid = fork();
	if (!tchat_pid)
		execl("/usr/bin/gnome-terminal", "/usr/bin/gnome-terminal", "--", "./modules/tchat/client", me.pseudo, NULL);
	wait(NULL); // Launched terminal exit immediatly, tchat client is not a child

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
	Client player_list[SERVER_MAX_CLIENTS];
	switch (message.command)
	{
		case VOTE:
			readPlayerList(message.data, player_list);
			printf("\nVote for (number) :\n");
			Client voted = chosePlayer(message.data, player_list);
			Message vote_message = {
				me,
				voted,
				VOTE
			};
			write(server_pipe_fd, &vote_message, sizeof(Message));
			break;
		
		case ASK_SPY:
			printf("\n%s\n", message.message);
			char word[STRING_MAX_SIZE] = {0};
			fgets(word, STRING_MAX_SIZE, stdin);
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
			printf("\nAsk a question to (number):\n");
			Client choosen = chosePlayer(message.data, player_list);
			Message question = {
				me,
				choosen,
				ASK_TO
			};
			
			printf("Question :\n");
			fgets(question.message,STRING_MAX_SIZE,stdin);
			write(server_pipe_fd, &question, sizeof(Message));
			break;
		case ANSWER:
			printf("\n%s ask you: %s\n",message.from.pseudo,message.message);						
			Message answer ={
				me,
				message.from,
				ANSWER
			};
			printf("Answer :\n");
			fgets(answer.message,STRING_MAX_SIZE,stdin);
			write(server_pipe_fd,&answer,sizeof(Message));
			break;

		case IS_SPY:
			system("clear");
			printf("You are the spy\n");
			break;

		case WORD:
			system("clear");
			printf("The word is: %s\n",message.message);
			break;

		default:
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
	char buff[STRING_MAX_SIZE] = {0};
	int choice = -1;
	for (int i = 0; i < n; i++)
	{
		printf("%d. %s\n", i+1, player_list[i].pseudo);
	}
	while (choice < 1 || choice > n)
	{
		printf("Choice :\n");
		fgets(buff, STRING_MAX_SIZE, stdin);
		choice = (int) strtol(buff, NULL, 10);
	}

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
