#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "server_config.h"
#include "structures.h"

int createServerPipe();
void writeServerInfo();
void handleMessage(Message*, Client*, int* connected_clients);
void handleConnection(Client*, Client*, int* connected_clients);
void handleDeconnection(Client*, Client*, int* connected_clients);

int server_pipe_fd = 0; 

int main()
{
	// Create server resources
	server_pipe_fd = createServerPipe();
	writeServerInfo();

	// Clients management variables
	int connected_clients = 0;
	Client clients[SERVER_MAX_CLIENTS];

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

int createServerPipe()
{
	mkfifo(SERVER_PIPE_NAME, 0666);
	return open(SERVER_PIPE_NAME, O_RDONLY);
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

	// MUST CHECK
	close(server_pipe_fd);
	server_pipe_fd = open(SERVER_PIPE_NAME, O_RDONLY);

	for (int i = 0; i < *connected_clients; i++)
	{
		if (clients[i].PID == client->PID)
		{
			for (int j = i; j < *connected_clients - 1; j++)
				clients[j] = clients[j+1];
		}
	}
	*connected_clients = *connected_clients - 1;

	printf("%s has disconnected.\n", client->pseudo);
	printf("\tNumber of client connected : %d\n", *connected_clients);
}
