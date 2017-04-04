#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include <commons/config.h>
#include "abstracciones.c"



int main(int argc, char**argv){
	archivoConfigKernel* t_archivoConfig = malloc(sizeof(archivoConfigKernel));
	t_config *config = malloc(sizeof(t_config));
	printf("arranquemos, so\n");
	configuracion(t_archivoConfig, config, argv[1]);

	return EXIT_SUCCESS;
}
void configuracion(archivoConfigKernel *unArchivo, t_config* config, char dir[]){
	config = config_create(dir);
	unArchivo->PUERTO_CPU = config_get_int_value(config, "PUERTO_CPU");
	printf("PUERTO_CPU: %d\n", unArchivo->PUERTO_CPU);
	unArchivo->PUERTO_PROG = config_get_int_value(config, "PUERTO_PROG");
	printf("PUERTO_PROG: %d\n", unArchivo->PUERTO_PROG);
	unArchivo->IP_MEMORIA  = config_get_int_value(config, "IP_MEMORIA");
	printf("IP_MEMORIA: %d\n", unArchivo->IP_MEMORIA);
	unArchivo->IP_FS = config_get_int_value(config, "IP_FS");
	printf("IP_FS: %d\n", unArchivo->IP_FS);
	unArchivo->PUERTO_FS  = config_get_int_value(config, "PUERTO_FS");
	printf("PUERTO_FS: %d\n", unArchivo->PUERTO_FS);
	unArchivo->QUANTUM = config_get_int_value(config, "QUANTUM");
	printf("QUANTUM:%d\n", unArchivo->QUANTUM);
	unArchivo->QUANTUM_SLEEP  = config_get_int_value(config, "QUANTUM_SLEEP");
	printf("QUANTUM_SLEEP:%d\n", unArchivo->QUANTUM_SLEEP );
	unArchivo->ALGORITMO  = config_get_string_value(config, "ALGORITMO");
	printf("ALGORITMO: %s\n", unArchivo->ALGORITMO);
	unArchivo->GRADO_MULTIPROG  = config_get_int_value(config, "GRADO_MULTIPROG");
	printf("GRADO_MULTIPROG:%d\n", unArchivo->GRADO_MULTIPROG);
	unArchivo->STACK_SIZE  = config_get_int_value(config, "STACK_SIZE");
	printf("STACK_SIZE:%d\n", unArchivo->STACK_SIZE);
	//unArchivo->SEM_IDS = config_get_array_value(config, "SEM_IDS");
	//unArchivo->SEM_INIT  = config_get_array_value(config, "SEM_INIT");
	//unArchivo->SHARED_VARS = config_get_array_value(config, "SHARED_VARS");
	//ojo ips, solo impre el primero. ojo arrays.
}
