#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define SERVER_PIPE_NAME "myfifo"
#define SERVER_INFO "servinfo"

int createServerPipe(char *);
void writeServerInfo(char * );


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
			char message[64] = {0};
			int byte_read = 0;

			while (1)
			{
				byte_read = read(client_pipe_fd, message, 64);
				message[byte_read] = 0;
				printf("[%s] %s\n", client_pid, message);
			}
			close(client_pipe_fd);
			break;
		}
	}

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

