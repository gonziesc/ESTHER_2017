#ifndef SERIALIZADOR_H_
#define SERIALIZADOR_H_

#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include "configuracion.h"

typedef struct{
	int32_t header;
	int32_t size;
	void* package;
}__attribute__((packed))paquete;

typedef struct {
	int pag;
	int size;
	int off;
}__attribute__((packed)) posicionMemoria;

typedef struct {
	int pos;
	t_list *args;
	t_list *vars;
	int retPos;
	posicionMemoria retVar;
	int tamanoArgs;
	int tamanoVars;
}__attribute__((packed)) indiceDeStack;

typedef struct variable
{
	char etiqueta;
	posicionMemoria *direccion;
}__attribute__((packed)) variable;


typedef struct {
	int32_t tamanoTotal;
	int32_t programId;
	int32_t programCounter;
	int32_t cantidadDePaginas;
	int32_t exitCode;
	int32_t tamanoIndiceCodigo;
	int32_t tamanoindiceEtiquetas;
	int32_t tamanoIndiceStack;
	int* indiceCodigo;
	char* indiceEtiquetas;
	t_list* indiceStack;
}__attribute__((packed)) programControlBlock;

void Serializar(int32_t , int32_t , void*, int32_t);
paquete* Deserializar(int32_t);



#endif
