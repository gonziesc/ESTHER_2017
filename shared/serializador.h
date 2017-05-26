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
#include "configuracion.h"

typedef struct{
	int32_t header;
	int32_t size;
	char* package;
}__attribute__((packed))
paquete;

void Serializar(int32_t , int32_t , void*, int32_t);
char* Deserializar(int32_t ,int32_t,int32_t*);



#endif
