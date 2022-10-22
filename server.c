#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

#include "server_config.h"
#include "structures.h"

void sigintHandler(int);
void sigusr1Handler(int);
void sigusr2Handler(int);

void serverMenu();
void startCountdown(int);

int createServerPipe();
void writeServerInfo();
void handleMessage(Message*, Client*, int* connected_clients);
void handleConnection(Client*, Client*, int* connected_clients);
void handleDeconnection(Client*, Client*, int* connected_clients);

// Server management variables
int server_pipe_fd = 0; 
pid_t server_menu_pid = 0;
int start_game = 0;

// Clients management variables
int connected_clients = 0;
Client clients[SERVER_MAX_CLIENTS];

int main()
{
	// Register signals
	signal(SIGINT, sigintHandler);
	signal(SIGUSR1, sigusr1Handler);
	signal(SIGUSR2, sigusr2Handler);

	// Create server resources
	server_pipe_fd = createServerPipe();
	writeServerInfo();

	// Server menu
	server_menu_pid = fork();
	if (!server_menu_pid)
	{
		serverMenu();
	}

	// Server pipes buffer
	Message message_buffer;

	int byte_read = 0;
	while (!start_game)
	{	
		// Wait for a message in server pipe
		byte_read = read(server_pipe_fd, &message_buffer, sizeof(Message));
		if (!byte_read || byte_read == EOF)
			continue;
		
		handleMessage(&message_buffer, clients, &connected_clients);
	}

	// Game init
	printf("GAME START\n");
	startCountdown(GAME_TIME_LIMIT);

	while (start_game)
	{
		
	}

	printf("GAME END\n");

	return 0;	
}

void sigintHandler(int signum)
{
	for (int i = 0; i < connected_clients; i++)
	{
		close(clients[i].pipe_fd);
		kill(clients[i].PID, SIGINT);
	}

	close(server_pipe_fd);
	unlink(SERVER_PIPE_NAME);

	wait(NULL);
	exit(0);
}

void sigusr1Handler(int signum)
{
	start_game = 1;
}

void sigusr2Handler(int signum)
{
	start_game = 0;
}

void serverMenu()
{
	for (int i = 0; i < SERVER_MENU_WIDTH; i++)
		printf("=");
	printf("\n");

	char* choice1_label = "| 1. Start game";
	printf("%s", choice1_label);
	for (int i = 0; i < SERVER_MENU_WIDTH - strlen(choice1_label) - 1; i++)
		printf(" ");
	printf("|\n");

	char* choice2_label = "| 2. Stop server";
	printf("%s", choice2_label);
	for (int i = 0; i < SERVER_MENU_WIDTH - strlen(choice2_label) - 1; i++)
		printf(" ");
	printf("|\n");

	for (int i = 0; i < SERVER_MENU_WIDTH; i++)
		printf("=");
	printf("\n\n");
	printf("Players :\n");

	int choice = 0;
	scanf("%d", &choice);

	switch(choice)
	{
		case 1:
			kill(getppid(), SIGUSR1);

			// Send start message to exit main process reading
			Client server = {"SERVER", -1, server_pipe_fd, getpid()};
			Message start_message = {server, START, "START"};
			write(server.pipe_fd, &start_message, sizeof(Message));

			break;
		case 2:
			kill(getppid(), SIGINT);
			break;
	}

	exit(0);
}

void startCountdown(int seconds)
{
	pid_t countdown_pid = fork();
	if (!countdown_pid)
	{
		time_t time1, time2;
		time1 = time(NULL);

		while (1)
		{
			sleep(1);
			time2 = time(NULL);
			if (difftime(time2, time1) >= seconds)
			{
				kill(getppid(), SIGUSR2);
				exit(0);
			}
		}
	}
}

int createServerPipe()
{
	unlink(SERVER_PIPE_NAME);
	mkfifo(SERVER_PIPE_NAME, 0666);
	return open(SERVER_PIPE_NAME, O_RDWR);
}

void writeServerInfo() {
	FILE* info_file = fopen(SERVER_INFO_FILE_PATH, "w");
	
	if (info_file != NULL)
	{
		fprintf(info_file, "%s", SERVER_PIPE_NAME);
		fclose(info_file);
		printf("Successfully created %s file.\n", SERVER_INFO_FILE_PATH);
		printf("\tServer pipe name is : %s\n\n", SERVER_PIPE_NAME);
	} 
	else 
	{
        	fprintf(stderr, "Error while creating %s file.\n", SERVER_INFO_FILE_PATH);
    	}
}

void handleMessage(Message* message, Client* clients, int* connected_clients)
{
	switch (message->command)
	{
		case CONNECTION:
			handleConnection(&message->client, clients, connected_clients);
			break;

		case DECONNECTION:
			handleDeconnection(&message->client, clients, connected_clients);
			break;

		case MESSAGE:
			// MUST MAKE A FUNCTION
			printf("[%s] %s\n", message->client.pseudo, message->message);
			for (int i = 0; i < *connected_clients; i++)
			{
				if (clients[i].PID != message->client.PID)
				{
					write(clients[i].pipe_fd, message, sizeof(Message));
				}
			}
			break;
	}
}

void handleConnection(Client* client, Client* clients, int* connected_clients)
{
	// Open client pipe
	char client_pipe_name[STRING_MAX_SIZE] = {0};
	sprintf(client_pipe_name, "%d", client->PID);
	client->pipe_fd = open(client_pipe_name, O_WRONLY);

	// Update client list
	clients[*connected_clients] = *client;
	*connected_clients = *connected_clients + 1;

	// Print information
	printf("\t%s\n", client->pseudo);
}

void handleDeconnection(Client* client, Client* clients, int* connected_clients)
{
	close(client->pipe_fd);

	// Must reopen server pipe, otherwise can't read anymore
	close(server_pipe_fd);
	server_pipe_fd = open(SERVER_PIPE_NAME, O_RDWR);

	for (int i = 0; i < *connected_clients; i++)
	{
		if (clients[i].PID == client->PID)
		{
			for (int j = i; j < *connected_clients; j++)
			{
				if (j != SERVER_MAX_CLIENTS) // Prevent seg fault
					clients[j] = clients[j+1];
				else
				{
					Client null_client;
					clients[j] = null_client;
				}
			}
		}
	}
	*connected_clients = *connected_clients - 1;

	printf("%s has disconnected.\n", client->pseudo);
	printf("\tNumber of client connected : %d\n", *connected_clients);
}
