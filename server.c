#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

#define SERVER_PIPE_NAME "myfifo"
#define SERVER_INFO "servinfo"
#define BDD_PATH "ressource/database.csv"
#define PSEUDO_MAX_SIZE 32

typedef struct Client
{
	char pseudo[PSEUDO_MAX_SIZE];
	int is_spy;
	int has_token;
	int vote;
	pid_t PID;
} Client;

int createServerPipe(char *);
void writeServerInfo(char * );
void searchAnswer(char*, char*, char*);

void printClient(Client);

int main(){
	int server_pipe_fd = createServerPipe(SERVER_PIPE_NAME);
	writeServerInfo(SERVER_INFO);

	pid_t client_handler_pid = 0;
	int client_pipe_fd = 0;

	char client_pid[64] = {0};
	while (1)
	{	
		read(server_pipe_fd, client_pid, 64);	//Attente pid client dans server pipe - read bloquant
		printf("%s\n", client_pid);
		client_handler_pid = fork();

		if (!client_handler_pid) //Si c' est le fils ( = 0)
		{
			client_pipe_fd = open(client_pid, O_RDWR); //buf = pid  client qui est aussi le nom du pipe avec lequel on communiquera au client
			printf("CLIENT PIPE FD : %d\n",client_pipe_fd);

			Client players[5] = {
				{"Andre", 0, 0, 0, 0},
				{"Olivier", 0, 0, 0, 0},
				{"Cyprien", 0, 0, 0, 0},
				{"Adnen", 1, 0, 0, 0},
				{"Gabriel", 0, 0, 0, 0}			
			};
			Client client_buffer;

			while (1)
			{
				printf("Press enter to send player list."); getchar();
				for (int i = 0; i < 5; i++)
					write(client_pipe_fd, &players[i], sizeof(Client));
				usleep(100);
				read(client_pipe_fd, &client_buffer, sizeof(Client));
				printClient(client_buffer);
			}
			close(client_pipe_fd);
			exit(0);
		}
	}
	
	wait(NULL);
	close(server_pipe_fd);
	unlink(SERVER_PIPE_NAME);
	return 0;	
}

int createServerPipe(char * myfifo)
{

    // FIFO file path

    // Creating the named file(FIFO)
    // mkfifo(<pathname>,<permission>)
    mkfifo(myfifo, 0666);

    
    return open(myfifo,O_RDWR);
}

void writeServerInfo(char * info_file_path) {
	FILE* fichier = NULL;// on cree un truc la

	pid_t pid = getpid ();

	printf ("PID: %d\n", pid);

	

	fichier = fopen(info_file_path, "w+");// on ouvre ou cree le fichier
	if (fichier != NULL)
	{
		printf("Ouverture ok\n");
		fprintf (fichier,"%s",SERVER_PIPE_NAME);
	} 
	else 
	{
        	printf("Impossible d'ouvrir le fichier test.txt");
    	}


	fclose(fichier);
}

void searchAnswer(char* question, char* answer, char* BDD)
{

	
	char lecteur[128];
	
	FILE* fd = fopen(BDD, "r");
	
	if(fd == NULL)
	{
		printf("[ERROR] Can't open %s\n", BDD);
		exit(1);
	}

	int next_line_is_answer = 0;	
	int byte_read = 0;
	while(fgets(lecteur, sizeof lecteur, fd) != NULL)
	{
		byte_read = strlen(lecteur);
		lecteur[byte_read] = 0; // remove trailing \n
		
		if (next_line_is_answer)
		{
			strcpy(answer, lecteur);
			break;
		}
	
		if (strcmp(question,lecteur) == 0)
		{
			next_line_is_answer = 1;
		}
		
	}
}

void printClient(Client client)
{
	printf("Pseudo : %s\n", client.pseudo);
	printf("Is spy : %s\n", client.is_spy ? "True" : "False");
}
