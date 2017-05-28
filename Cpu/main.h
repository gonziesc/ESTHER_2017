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
#include <parser/parser.h>
#include <serializador.h>

void Configuracion(char *);
int32_t ConectarConKernel();
int32_t conectarConMemoria();

t_puntero dummy_definirVariable(t_nombre_variable variable);
t_puntero dummy_obtenerPosicionVariable(t_nombre_variable variable);
t_valor_variable dummy_dereferenciar(t_puntero puntero);
void dummy_asignar(t_puntero puntero, t_valor_variable variable);
void dummy_finalizar(void);
char* depurarSentencia(char*);

bool terminoElPrograma(void);

#endif
