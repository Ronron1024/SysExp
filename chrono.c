#include <stdio.h>
#include <stdlib.h>
#include <time.h>//A ne pas oublier
#include <unistd.h>//Pour le sleep

int main (void)
{
	time_t temps1, temps2, temps3;//Les variables de temps sont de type time_t
	 
	temps1 = time(NULL);//On donne une valeur de temps à temps1.
	//sleep(2);//On attend 2 secondes.
	//temps2 = time(NULL);//On donne une valeur de temps à temps2.
	//printf("difference entre temps1 et temps2 : %lf", difftime(temps2, temps1));//On affiche la difference de temps entre temps 1 et temps2
	while(1)
	{
		sleep(0);//On attend 1 secondes.
		temps2 = time(NULL);//On donne une valeur de temps à temps2.
		//system("clear"); /*clear output screen*/
		printf("%lf\r",difftime(temps2, temps1));
			if (difftime(temps2, temps1) >= 900) {
				printf("15 mins ecoulées");
				return 10;
			} else {
				sleep(0);
				temps3 = time(NULL);
				printf("%lf\r",difftime(temps2, temps1)); }

	}
	 
	return EXIT_SUCCESS;
}
