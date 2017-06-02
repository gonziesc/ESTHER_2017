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



#endif
