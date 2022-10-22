#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>

#define NAME_PIPE_CLIENT 8

#define MAX 15

int countlines(char *filename);
int readServerInfo(char* info_file_path);
char PickRandom(char* info_file_path ,int taille);


int createClientPipe();
// create a named pipe for communicate with the server and return its file descriptor.
// Name of the pipe is PID.

int main (int argc, char *agv[]){

int i;
    int desc=0;
    char* info_file_path ="servinfo";
   i=countlines(info_file_path);
  printf("nb lignes: %d\n", i);
   
   
    desc = PickRandom(info_file_path,i);
 //   write(desc, "coucou ",7) ;
    close(desc);
}


int readServerInfo(char* info_file_path){

    int dp=0;
    int desc=0;
    FILE* file = fopen(info_file_path, "r");
    char buf[MAX];
    fgets(buf, MAX, file);

    printf("pipe is: %s\n", buf);

    dp=open(buf,O_WRONLY);
    close(desc);
    return dp;
}


char PickRandom(char* info_file_path ,int taille){

    int dp=0;
    int desc=0;

    srand(time(NULL));
    int i ;
    taille = rand() % (taille);
   
    printf("random: %d\n", taille);
   
   
   
   
    FILE* file = fopen(info_file_path, "r");   // on ouvre le fichier en lecture
    char buf[MAX];                              //
                                                //
    for (i=0 ; i<taille; i++)     // cette boucle nous permet d'avancer jusqu'a la ligne souhaité
    {                                           //
fgets(buf, MAX, file); //  
}

    printf("pipe is: %s\n", buf);
     dp=open(buf,O_WRONLY);
    close(desc);
    return buf;
}




int countlines(char *filename)
{
  // count the number of lines in the file called filename                                    
  FILE *fp = fopen(filename,"r");
  int ch=0;
  int lines=0;