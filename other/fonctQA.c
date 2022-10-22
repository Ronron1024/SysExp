#include<stdlib.h>
#include<stdio.h>
#include <string.h>


void searchAnswer(char* question, char* answer, char* BDD)
{

	
	char lecteur[128];
	
	FILE* fd = fopen(BDD, "r");
	
	if(fd == NULL)
	{
		printf("pas de question ni de reponse ici\n");
		exit(0);
	}

	int next_line_is_answer = 0;	
	while(fgets(lecteur, sizeof lecteur, fd) != NULL)
	{
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



int main (void)
{
	char answer[128];
	searchAnswer("le lieu se trouve sur terre mer ou ciel\n", answer, "database.csv");
	printf("%s\n", answer);
}


