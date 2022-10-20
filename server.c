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
#define NUMBER_CLIENT_MAX 64
#define CHAR_NAME_MAX 16
#define NAME_PIPE_CLIENT 8

typedef struct client 
{ 
	char name[CHAR_NAME_MAX];
	int is_spy;
	int token; //pid de celui qui interroge
	char pid[NAME_PIPE_CLIENT];
	int vote;
} CLIENT;

int createServerPipe(char *);
void writeServerInfo(char * );
void searchAnswer(char*, char*, char*);

void printClient(CLIENT);

int main(){

	CLIENT Client[NUMBER_CLIENT_MAX];
	CLIENT *Player;

	int nbclient = 0;
	int go =0;
	int choix = 0;

	int server_pipe_fd = createServerPipe(SERVER_PIPE_NAME);
	if (server_pipe_fd == -1)
	{
		printf(" Err open pipe: server_pipe_fd\n");
	} 

	writeServerInfo(SERVER_INFO);
    pid_t client_handler_pid = 0;
	int client_pipe_fd = 0;

	char client_pid[64] = {0};

	/*Menu*/

	pid_t menu= fork();
	
	if(!menu)
	{	
		printf("---------------\n");
		printf("|1-Start Game |\n");
		printf("|2-Stop Server|\n");
		printf("|Choix?       |\n");
		printf("---------------\n");

		while (choix == 0)
		{

			switch ( getchar() )
			{

				case '1':
					printf("Choix 1\n");
					choix = 1;
					//Random word/spy/token
					go = 1;					
					break;

				case '2':
					printf("choix 2\n");
					choix = 2;
					//Kill child
					break;

				defaut:
					break;
			}
		}

		exit(0);
	}
	
	/*End Menu*/
	while(1)
	{
		if (go)//Game loop
		{
			//Init game envoie word et et spy
			//Envoie la liste des client à celui qui à le token pour parler
			//Question client vers client
			//Affichage reponse sur serveur
			//
		}

		if ( read(server_pipe_fd,&Client[nbclient],sizeof(CLIENT)) == -1 )
		{
			printf("Err read server pipe\n");
		}

		printClient( Client[nbclient] );
		open(Client[nbclient].pid, O_RDWR);


		/*Communication à plusieur pas necessaire - fonction tchat à part
 
		client_handler_pid = fork();

		if (!client_handler_pid) //Si c' est le fils ( = 0)
		{
			client_pipe_fd = open(Client[nbclient].pid, O_RDWR); //buf = pid  client qui est aussi le nom du pipe avec lequel on communiquera au client
			printf("CLIENT PIPE FD : %d\n",client_pipe_fd);
			char message[64] = {0};
			char answer[64]={0};
			int byte_read = 0;

			CLIENT client_buffer;
			while (1)
			{
				printf("Press enter to send player list."); getchar();
				write(client_pipe_fd, &Client[nbclient], sizeof(CLIENT));
				usleep(100);
				read(client_pipe_fd, &client_buffer, sizeof(CLIENT));
				printClient(client_buffer);
            		}
			close(client_pipe_fd);
			exit(0);
		}
		*/
		nbclient++;
	}

	printf("PAPA est la!! wait = %d\n", wait(NULL));
	while ( wait(NULL) != -1);
	printf("BYE");

	close(server_pipe_fd);
	unlink(SERVER_PIPE_NAME);
	return 0;	
}

int createServerPipe(char * myfifo)
{

    // FIFO file path

    // Creating the named file(FIFO)
    // mkfifo(<pathname>,<permission>)
    unlink(myfifo);
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

void printClient(CLIENT client)
{
	printf("Pseudo : %s\n", client.name);
	printf("Is spy : %s\n", client.is_spy ? "True" : "False");
}
