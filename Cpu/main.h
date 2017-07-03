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

int programaBloqueado;
int programaFinalizado;
int programaAbortado;

void Configuracion(char *);
int32_t ConectarConKernel();
int32_t conectarConMemoria();

t_descriptor_archivo abrir(t_direccion_archivo , t_banderas );
void borrar (t_descriptor_archivo );
void cerrar(t_descriptor_archivo );
void leer(t_descriptor_archivo , t_puntero , t_valor_variable );
void moverCursor (t_descriptor_archivo , t_valor_variable );



void wait_kernel(t_nombre_semaforo );
void signal_kernel(t_nombre_semaforo );
void liberar(t_puntero ) ;
t_puntero reservar(t_valor_variable );

t_puntero definirVariable(t_nombre_variable );
t_puntero obtenerPosicionVariable(t_nombre_variable );
t_valor_variable dereferenciar(t_puntero );
void enviarDirecParaEscribirMemoria( posicionMemoria* , int );
void proximaDireccion(int , int , posicionMemoria* );
void armarProximaDireccion(posicionMemoria* );
void armarDireccionPrimeraPagina(posicionMemoria *);
void crearEstructuraParaMemoria(programControlBlock*, int, posicionMemoria *);
void asignar(t_puntero , t_valor_variable );
void finalizar(void);
char* depurarSentencia(char*);
char* leerSentencia(int , int , int, int );
int primeraPagina();
void procesar(char * , int32_t , int32_t );
void enviarDirecParaLeerMemoria(posicionMemoria* , int);
void convertirPunteroADireccion(int , posicionMemoria* );
int convertirDireccionAPuntero(posicionMemoria* );
bool terminoElPrograma(void);
void procesarScript();
void escribir(t_descriptor_archivo , void* , t_valor_variable );
t_valor_variable asignarValorCompartida(t_nombre_compartida ,t_valor_variable );
t_valor_variable obtenerValorCompartida(t_nombre_compartida );
void llamarConRetorno(t_nombre_etiqueta, t_puntero);
void llamarSinRetorno(t_nombre_etiqueta etiqueta);
void irAlLabel(t_nombre_etiqueta);
void retornar(t_valor_variable);

#endif
