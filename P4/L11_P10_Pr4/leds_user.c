#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

static int fichero;

int main(){

	fichero = open("/dev/chardev_leds", O_RDWR);

	printf("\nDamos por comenzada la secuencia de los leds: \n");

	int fin = 30000,i = 0;

	write(fichero, "", 0); //Inicializamos todos a 0 por si las moscas.

	while(i < fin){
		printf("Paso %d: \n", i); // Quitar luego si no quieres tener 30000 prints

		if(i%7 == 0) write(fichero, "1", 1);
		else if(i%7 == 1) write(fichero, "2", 1);
		else if(i%7 == 2) write(fichero, "3", 1);
		else if(i%7 == 3) write(fichero, "12", 2);
		else if(i%7 == 4) write(fichero, "13", 2);
		else if(i%7 == 5) write(fichero, "23", 2);
		else write(fichero, "123", 3);
	i++;
	}

	write(fichero, "", 0); //Dejamos todos a 0;

	close(fichero);

	return 0;
}