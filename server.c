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

void sendVoteMessage();
void sendPlayerList();
void updateVote(Client);
Client calcVote();
void askWord(Client);
GAME_RESULT getResult(Client, Client, char*, char*);

// Server management variables
int server_pipe_fd = 0; 
pid_t server_menu_pid = 0;
int start_game = 0;

// Clients management variables
int connected_clients = 0;
Client clients[SERVER_MAX_CLIENTS];


//Random
void PickRandom(char* info_file_path ,int taille, char* buf);
int chooseRandomInt(int connected_client);
int countlines(char *filename);


void sendPlayerListTo(Client addressee);
void sendAskToMessage(Client addressee);

int main()
{
	srand( time( NULL ) );

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
	int randomSpy = chooseRandomInt(connected_clients);
	Client spy = clients[randomSpy];
	clients[randomSpy].is_spy = 1;

	//printf("Le randomSpy is %d\n",randomSpy);
	//Server envoie spy au spy

	char word[STRING_MAX_SIZE];
	int numberWord = countlines(PATH_BDD_WORD);
	PickRandom(PATH_BDD_WORD,numberWord,word);

	printf("The word is = %s\n", word);
	//Server envoie le mot à tous sauf au spy
	
	int firstPlayer = chooseRandomInt(connected_clients);
	while( clients[firstPlayer].is_spy == 1){
		firstPlayer = chooseRandomInt(connected_clients);
	}

	pid_t token = clients[firstPlayer].PID;

	printf("token du first player = %d\n",token);
	printf("FirstPlayer= %s\n", clients[firstPlayer].pseudo);

	startCountdown(GAME_TIME_LIMIT);


	while (start_game)
	{
		sendAskToMessage(clients[firstPlayer]);
	 	sendPlayerListTo(clients[firstPlayer]);
		getchar();
		//Listes de joueurs
	 //choix du jour pour la question
	 //Envoie question
	 //cght token
	 //reponse
	}

	// END GAME
	sendVoteMessage();
	sendPlayerList();
	int has_voted = 0;
	while (has_voted < connected_clients)
	{
		read(server_pipe_fd, &message_buffer, sizeof(Message));
		
		if (message_buffer.command == VOTE)
		{
			printf("%s has voted %s\n", message_buffer.from.pseudo, message_buffer.to.pseudo);
			updateVote(message_buffer.to);
			has_voted++;
		}	
	}

	// Print vote result
	Client voted = calcVote();
	printf("\nVote result :  %s\n", voted.pseudo);
	printf("The spy was %s\n", spy.pseudo);

	// Ask the spy for word
	askWord(spy);
	read(server_pipe_fd, &message_buffer, sizeof(Message));
	char spy_word[STRING_MAX_SIZE] = {0};
	strcpy(spy_word, message_buffer.message);

	printf("%s says the word was %s.\n", spy.pseudo, spy_word);
	printf("\nThe word was %s\n", word);

	// Game result
	GAME_RESULT result = getResult(spy, voted, word, spy_word);
	switch (result)
	{
		case SPY:
			printf("The spy win !\n");
			break;
		case PLAYERS:
			printf("Players win !\n");
			break;
		case PAR:
			printf("Nobody win ...\n");
			break;
	}
		
	
	kill(getpid(), SIGINT); // EXIT BY SENDING SIGINT
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
			Message start_message = {server, server, START, "START"};
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
			handleConnection(&message->from, clients, connected_clients);
			break;

		case DECONNECTION:
			handleDeconnection(&message->from, clients, connected_clients);
			break;

		case MESSAGE:
			// MUST MAKE A FUNCTION
			printf("[%s] %s\n", message->from.pseudo, message->message);
			for (int i = 0; i < *connected_clients; i++)
			{
				if (clients[i].PID != message->from.PID)
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

void sendVoteMessage()
{
	Client server = {
		"SERVER",
		-1,
		server_pipe_fd,
		getpid()
	};
	Message vote_message = {
		server,
		server,
		VOTE,
		"VOTE",
		connected_clients - 1 // minus current player
	};

	for (int i = 0; i < connected_clients; i++)
	{
		vote_message.to = clients[i];
		write(clients[i].pipe_fd, &vote_message, sizeof(Message));
	}
}

void sendPlayerList()
{
	Message message_buffer;
	message_buffer.command = MESSAGE;

	for (int i = 0; i < connected_clients; i++)
	{
		message_buffer.from = clients[i];
		for (int j = 0; j < connected_clients; j++)
		{
			if (clients[j].PID != clients[i].PID)
			{
				message_buffer.to = clients[j];
				write(clients[j].pipe_fd, &message_buffer, sizeof(Message));
			}
		}
	}
}

void updateVote(Client is_voted)
{
	for (int i = 0; i < connected_clients; i++)
	{
		if (clients[i].PID == is_voted.PID)
			clients[i].vote++;
	}
}

Client calcVote()
{
	Client voted = clients[0];
	for (int i = 0; i < connected_clients; i++)
	{
		if (clients[i].vote > voted.vote)
			voted = clients[i];
	}

	return voted;
}

void askWord(Client client)
{
	Client server = {
		"SERVER",
		-1,
		server_pipe_fd,
		getpid()
	};
	Message ask_message = {
		server,
		client,
		ASK
	};
	sprintf(ask_message.message, "%s, what is the word ?", client.pseudo);
	printf("\n%s\n", ask_message.message);

	write(client.pipe_fd, &ask_message, sizeof(Message));
}

GAME_RESULT getResult(Client spy, Client voted, char* word, char* spy_word)
{
	if (strcmp(word, spy_word) == 0)
		return SPY;
	else if (spy.PID == voted.PID)
		return PLAYERS;
	else
		return PAR;
}




void PickRandom(char* info_file_path ,int taille, char* buf){

    	int dp=0;
    	int desc=0;
    	srand(time(NULL));

    	int i ;
    	taille = taille - rand() % (taille);
	printf("random: %d\n", taille);

	FILE* file = fopen(info_file_path, "r");   // on ouvre le fichier en lecture
	
	if(file == NULL)
		printf("Err acces bdd_word\n");

	for (i=0 ; i<taille; i++)     // cette boucle nous permet d'avancer jusqu'a la ligne souhait�
	{                                           //
		fgets(buf, sizeof(char)*STRING_MAX_SIZE, file); //
	}

//	printf("pipe is: %s\n", buf);
//	dp=open(buf,O_WRONLY);
//	close(desc);
}

int chooseRandomInt(int connected_clients)
{
	return (rand() % connected_clients);
}

int countlines(char *filename)
{

  // count the number of lines in the file called filename

  	FILE *fp = fopen(filename,"r");

	if (fp == NULL)
		printf("Err acces file\n");
	
	//char bufferLine[STRING_MAX_SIZE];
	char c = '\0';
  	int lines = 0;

	fseek(fp,0,SEEK_SET);

	while (c != EOF)
	{	
		while( c != '\n')
		{
			c = fgetc(fp);
		}
	c = fgetc(fp);
	lines++; 
	}

	printf("Nb line = %d\n",lines);

	return (chooseRandomInt(lines) + 1);
	
	//size_t fread( void * buffer, size_t blocSize, size_t blocCount, FILE * stream );  
}

void sendPlayerListTo(Client addressee)
{
	Message message_buffer;
	message_buffer.command = ASK_TO;

	for (int i = 0; i < connected_clients; i++)
	{
		if (addressee.PID != clients[i].PID)
		{
			message_buffer.from = clients[i];
			message_buffer.to = addressee;
			write(addressee.pipe_fd, &message_buffer, sizeof(Message));
			printf("How many/ name = %s\n",clients[i].pseudo);
		}
	}
}

void sendAskToMessage(Client addressee)
{
	Client server = {
		"SERVER",
		-1,
		server_pipe_fd,
		getpid()
	};
	Message vote_message = {
		server,
		server,
		ASK_TO,
		"ASK_TO",
		connected_clients - 1 // minus current player
	};

	for (int i = 0; i < connected_clients; i++)
	{	
		if(addressee.PID==clients[i].PID)
		{
			vote_message.to = addressee;
			write(addressee.pipe_fd, &vote_message, sizeof(Message));
		}
	}
}

