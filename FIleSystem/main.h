#ifndef MAIN_H_
#define MAIN_H_



#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include <commons/config.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <conexiones.c>
#include <configuracion.h>



void configuracion(char*);
int32_t levantarConexion();






#endif
