#ifndef MAIN_H_
#define MAIN_H_

#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include <commons/config.h>
#include <commons/temporal.h>
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
#include <semaphore.h>
#include <commons/log.h>

t_log* log;

typedef struct{
	int PID;
	int identificadorHilo;
	char* horaInicio;
	int cantidadDeImpresiones;
}ProcesosActuales;

void Configuracion(char *);
int32_t ConectarseConKernel();
void crearNuevoProceso();
int abrirYLeerArchivo(char*, char*);
void leerComando();
ProcesosActuales buscarProceso(int );
void matarTodosLosProcesos();
void imprimioProceso(int);



#endif
