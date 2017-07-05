#ifndef MAIN_H_
#define MAIN_H_



#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/io.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <commons/config.h>
#include <commons/bitarray.h>
#include <commons/string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <conexiones.c>
#include <configuracion.h>
#include <serializador.h>

#include <pthread.h>


void configuracion(char*, char*);
int32_t levantarConexion();






#endif
