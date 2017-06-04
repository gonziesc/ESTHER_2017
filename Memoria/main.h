#ifndef MAIN_H_
#define MAIN_H_

#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include <commons/config.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <conexiones.c>
#include <configuracion.h>
#include <pthread.h>
#include <serializador.h>
#include <semaphore.h>
#include <commons/log.h>

typedef struct {
	int32_t id;
	int32_t tamanio;
	int32_t tamanioDisponible;
	int32_t tamanioOcupado;
	char* puntero;
	char * punteroDisponible;
} frame;


typedef struct {
	int32_t pid;
	//int32_t puntero;
	int32_t numeroPagina;
} infoTablaMemoria;

typedef struct{
	int32_t tamanio;
	int32_t tamanioDisponible;
}cache;



void configuracion(char*);
int32_t levantarConexion();
void crearFrameGeneral();
void almacernarPaginaEnFrame(int32_t, int32_t, char*);
void procesar(char *, int32_t , int32_t,int32_t);
void dump();
void leerComando();
int32_t buscarFrame(int32_t, int32_t);
char* leerDePagina(int32_t , int32_t , int32_t , int32_t );
int atenderCpu(int);
int atenderKernel();
void conectarseConKernel();
void atenderConexionesCPu();
void escribirEnPagina(int32_t , int32_t , int32_t , int32_t ,char* );
void size();



#endif
