#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "server_config.h"
#include "structures.h"

void sigintHandler(int);

int createServerPipe();
void writeServerInfo();
void handleMessage(Message*, Client*, int* connected_clients);
void handleConnection(Client*, Client*, int* connected_clients);
void handleDeconnection(Client*, Client*, int* connected_clients);

int server_pipe_fd = 0; 

// Clients management variables
int connected_clients = 0;
Client clients[SERVER_MAX_CLIENTS];

int main()
{
	// Register SIGINT
	signal(SIGINT, sigintHandler);

	// Create server resources
	server_pipe_fd = createServerPipe();
	writeServerInfo();

	// Server pipes buffer
	Message message_buffer;

	int byte_read = 0;
	while (1)
	{	
		// Wait for a message in server pipe
		byte_read = read(server_pipe_fd, &message_buffer, sizeof(Message));
		if (!byte_read || byte_read == EOF)
			continue;
		
		handleMessage(&message_buffer, clients, &connected_clients);
	}

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

	exit(0);
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
		printf("\tServer pipe name is : %s\n", SERVER_PIPE_NAME);
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
	printf("%s has connected.\n", client->pseudo);
	printf("\tNumber of client connected : %d\n", *connected_clients);
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
