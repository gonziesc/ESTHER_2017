#ifndef MAIN_H_
#define MAIN_H_

#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include <string.h>
#include <commons/config.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <conexiones.c>
#include <configuracion.h>
#include <parser/parser.h>
#include <serializador.h>
#include <pthread.h>
#include <semaphore.h>

void Configuracion(char *);
int32_t ConectarConKernel();
int32_t conectarConMemoria();

t_puntero dummy_definirVariable(t_nombre_variable );
t_puntero dummy_obtenerPosicionVariable(t_nombre_variable );
t_valor_variable dummy_dereferenciar(t_puntero );
void enviarDirecParaEscribirMemoria( posicionMemoria* , int );
void proximaDireccion(int , int , posicionMemoria* );
void armarProximaDireccion(posicionMemoria* );
void armarDireccionPrimeraPagina(posicionMemoria *);
void crearEstructuraParaMemoria(programControlBlock*, int, posicionMemoria *);
void dummy_asignar(t_puntero , t_valor_variable );
void dummy_finalizar(void);
char* depurarSentencia(char*);
char* leerSentencia(int , int , int, int );
int primeraPagina();
void procesar(char * paquete, int32_t id, int32_t tamanoPaquete);
void enviarDirecParaLeerMemoria(posicionMemoria* direccion);
void convertirPunteroADireccion(int puntero, posicionMemoria* direccion);
int convertirDireccionAPuntero(posicionMemoria* direccion);
bool terminoElPrograma(void);
void procesarScript();


#endif
