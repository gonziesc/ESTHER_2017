#ifndef MAIN_H_
#define MAIN_H_

#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include <commons/config.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <conexiones.c>
#include <configuracion.h>
#include <pthread.h>
#include <serializador.h>
#include <parser/metadata_program.h>

typedef struct{
	int PID;
	int identificadorHilo;
}ProcesosActuales;

void Configuracion(char *);
int32_t ConectarseConKernel();
void crearNuevoProceso();
int abrirYLeerArchivo(char*, char*);
void leerComando();




#endif
