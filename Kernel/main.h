#ifndef MAIN_H_
#define MAIN_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
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
#include <pthread.h>
#include <semaphore.h>
#include <commons/log.h>
#include <stdbool.h>

typedef struct {
  int socketCPU;
  int socketCONSOLA;
  programControlBlock *pcb;
  bool abortado;
} proceso;

typedef struct {
  int tamano;
  char* codigo;
  int socket;
} script;

typedef struct {
  int tamanoDisponible;
  int pid;
  int numeroPagina;
  int cantidadDeAlocaciones;
} datosAdminHeap;

typedef struct {
  int pid;
  int cantidad;
  int socket;
} hiloHeap;

typedef struct {
  int quantum;
  int quantumSleep;
  int algoritmo;
  int stack;
} datosKernelACpu;

typedef struct {
int size;
int isFree;
}__attribute__((packed)) HeapMetaData;

typedef struct {
	int pagina;
	int offset;
}datosHeap;

typedef struct {
	int pagina;
	int offset;
	int pid;
	int socket;
}liberaDatosHeap;

typedef struct {
int pid;
int consola;
}procesoConsola;

typedef struct {
int pid;
char* semaforo;
}procesoBloqueado;



void liberarHeap();
void procesarHeap();
void procesoLiberaHeap(int, int, int);
datosHeap* procesoPideHeap(int , int );
int pideVariable(char*);
void escribeVariable(char*, int);
void configuracion(char*);
void planificadorLargoPlazo();
int32_t conectarConMemoria();
int32_t ConectarConFS();
int32_t levantarServidor();
char* empaquetarPagina(int, char *, int);
void ejecutar(proceso* , int );
void planificadorCortoPlazo();
proceso* sacarProcesoDeEjecucion(int );
void procesarScript();
int existePaginaParaPidConEspacio(int, int);
void pedirAMemoriaUnaPaginaPara(int, int);
void guardarPaginaEnTabla(int, int, int);
int actualizarPaginaEnMemoria(char*, int, int, int);
int crearPaginaEnMemoria( int, int, int);
void pedirAMemoriaElPunteroDeLaPaginaDondeEstaLibre(int, int) ;
void abortarProgramaPorConsola(int, int);
proceso* sacarProcesoDeEjecucionPorPid(int );
void abortarTodosLosProgramasDeConsola(int);
char* conseguirSemaforoDeBloqueado(int);


#endif
