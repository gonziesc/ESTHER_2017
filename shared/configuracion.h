#ifndef CONFIGURACION_H_
#define CONFIGURACION_H_

#include <commons/config.h>
#include<stdio.h>
#include<stdlib.h>
#include <string.h>

typedef struct{
	int PUERTO;
	int MARCOS;
	int MARCOS_SIZE;
	int ENTRADAS_CACHE;
	int CACHE_X_PROC;
	char* REEMPLAZO_CACHE;
	int RETARDO_MEMORIA;
}archivoConfigMemoria;

typedef struct{
	int PUERTO_PROG;
		int PUERTO_CPU;
		char* IP_MEMORIA;
		int PUERTO_MEMORIA;
		char* IP_FS;
		int PUERTO_FS;
		int QUANTUM;
		int QUANTUM_SLEEP;
		char* ALGORITMO;
		int GRADO_MULTIPROG;
		char** SEM_IDS; //ojo memoria
		char** SEM_INIT;
		char** SHARED_VARS;
		int STACK_SIZE;
}archivoConfigKernel;

typedef struct{
	char *IP_KERNEL;
	int PUERTO_KERNEL;
}archivoConfigCPU;


typedef struct{
	char *IP_KERNEL;
	int PUERTO_KERNEL;
}archivoConfigConsola;


typedef struct{
	char *PUERTO_MONTAJE;
	int PUERTO_KERNEL;
}archivoConfigFS;

void configuracionMemoria(archivoConfigMemoria *, t_config* , char *);
void configuracionKernel(archivoConfigKernel *, t_config* , char *);
void configuracionConsola(archivoConfigConsola *, t_config* , char *);
void configuracionFS(archivoConfigFS *, t_config* , char *);
void configuracionCpu(archivoConfigCPU *, t_config* , char *);

#endif
