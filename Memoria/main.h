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





void configuracion(char*);
int32_t levantarConexion();
void crearFrame();
void almacenarArchivo(char,int32_t,int32_t);
void procesar(char *, int32_t , int32_t,int32_t);






#endif
