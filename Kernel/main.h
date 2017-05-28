#ifndef MAIN_H_
#define MAIN_H_

#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <unistd.h>
#include <math.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <conexiones.c>
#include <configuracion.h>
#include <serializador.h>
#include <parser/metadata_program.h>
#include <parser/parser.h>

void configuracion(char*);
int32_t conectarConMemoria();
int32_t ConectarConFS();
int32_t levantarServidor();
char* empaquetarPagina(int, char *, int);

typedef struct {
	int pag;
	int pos;
	int off;
} posicionMemoria;

typedef struct {
	int pos;
	t_list *args;
	t_list *vars;
	int retPos;
	posicionMemoria retVar;
} indiceDeStack;

typedef struct {
	int32_t programId;
	int32_t programCounter;
	int32_t cantidadDePaginas;
	int32_t exitCode;
	int* indiceCodigo;
	char* indiceEtiquetas;
	t_list* indiceStack;
}__attribute__((packed)) programControlBlock;

#endif
