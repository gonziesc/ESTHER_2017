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
#include <commons/collections/list.h>

typedef struct {
	int32_t id;
	int32_t tamanio;
	int32_t tamanioDisponible;
	int32_t tamanioOcupado;
	int32_t framesLibres;
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
	int32_t framesLibres;
	char* puntero;
	char*punteroDisponible;
}cache;

typedef struct{
	int32_t pid;
	int32_t numeroPagina;
	int32_t inicioContenido;

}infoNodoCache;

typedef struct{
	int32_t pid;
	int32_t pagina;
	int32_t uso;
}cacheLru;



void configuracion(char*);
int32_t levantarConexion();
void crearFrameGeneral();
void crearCache();
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
void liberarPaginaDeProceso(int32_t , int32_t );
int32_t buscarPidCache(int32_t);
void remplazoLru(infoNodoCache,char*);
int32_t buscarNodoCache(int32_t , int32_t );
void ordenarPorUso();
char* leerDeCache(int32_t , int32_t ,int32_t, int32_t );
void escribirEnCache(int32_t , int32_t , int32_t , int32_t , char* );
void almacenarFrameEnCache(int32_t , int32_t , char* , int32_t);
int32_t buscarPosicionContenido(int32_t, int32_t );
void inicializarPrograma(int32_t, int32_t );
void inicializarMemoria();
int32_t estaLibre(int32_t);
int32_t buscarUltimaPag(int32_t);
int32_t buscarFrameLibre();
void asignarPaginasAProceso(int, int);

t_list** overflow;
int CANTIDAD_DE_MARCOS;
unsigned int calcularPosicion(int pid, int num_pagina);
void inicializarOverflow(int cantidad_de_marcos);
void agregarSiguienteEnOverflow(int pos_inicial, int nro_frame);
int buscarEnOverflow(int indice, int pid, int pagina);
void borrarDeOverflow(int pos_inicial, int frame);
int esPaginaCorrecta(int pos_candidata, int pid, int pagina);


#endif

