#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define SERVER_PIPE_NAME "myfifo"
#define SERVER_INFO "serverinfo"

int createServerPipe(char *);
void writeServerInfo(char * );


int main(){
	createServerPipe(SERVER_PIPE_NAME);
	writeServerInfo(SERVER_INFO);
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

	printf ("PID         : %ld\n", pid);

	

	fichier = fopen(info_file_path, "w+");// on ouvre ou cree le fichier
	if (fichier != NULL){
        // On peut lire et écrire dans le fichier
	} else {
        // On affiche un message d'erreur si on veut
        printf("Impossible d'ouvrir le fichier test.txt");
    	}
	fprintf (fichier,"%s",SERVER_PIPE_NAME);

	fclose(fichier);
}

