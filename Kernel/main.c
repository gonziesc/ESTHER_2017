#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include <commons/config.h>



int main(int argc, char**argv){
	int puerto;
	t_config *config;
	printf("arranquemos, so");

	config = config_create(argv[1]);

	puerto = config_get_int_value(config, "PUERTO_PROG");
	printf("%d", puerto);
	return EXIT_SUCCESS;
}
