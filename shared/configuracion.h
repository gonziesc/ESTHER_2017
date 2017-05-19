#ifndef CONFIGURACION_H_
#define CONFIGURACION_H_

#include <commons/config.h>
#include<stdio.h>
#include<stdlib.h>
#include <string.h>

typedef enum {
	OK = 0,
	ARCHIVO = 1,
	PCB = 2,
	MEMORIA = 3,
	FILESYSTEM = 4,
	KERNEL = 5,
	CPU = 6,
	CONSOLA = 7,
	CODIGO = 8,
	TAMANO = 9
} codigosSerializador;

typedef struct{
	int32_t PUERTO;
	int32_t MARCOS;
	int32_t MARCOS_SIZE;
	int32_t ENTRADAS_CACHE;
	int32_t CACHE_X_PROC;
	char* REEMPLAZO_CACHE;
	int32_t RETARDO_MEMORIA;
}archivoConfigMemoria;

typedef struct{
	int32_t PUERTO_PROG;
	int32_t PUERTO_CPU;
		char* IP_MEMORIA;
		int32_t PUERTO_MEMORIA;
		char* IP_FS;
		int32_t PUERTO_FS;
		int32_t QUANTUM;
		int32_t QUANTUM_SLEEP;
		char* ALGORITMO;
		int32_t GRADO_MULTIPROG;
		char** SEM_IDS; //ojo memoria
		char** SEM_INIT;
		char** SHARED_VARS;
		int32_t STACK_SIZE;
}archivoConfigKernel;

typedef struct{
	char *IP_KERNEL;
	int32_t PUERTO_KERNEL;
	char *IP_MEMORIA;
	int32_t PUERTO_MEMORIA;
}archivoConfigCPU;


typedef struct{
	char *IP_KERNEL;
	int32_t PUERTO_KERNEL;
}archivoConfigConsola;


typedef struct{
	char *PUERTO_MONTAJE;
	int32_t PUERTO_KERNEL;
}archivoConfigFS;

void configuracionMemoria(archivoConfigMemoria *, t_config* , char *);
void configuracionKernel(archivoConfigKernel *, t_config* , char *);
void configuracionConsola(archivoConfigConsola *, t_config* , char *);
void configuracionFS(archivoConfigFS *, t_config* , char *);
void configuracionCpu(archivoConfigCPU *, t_config* , char *);

#endif
