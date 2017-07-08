#ifndef MAIN_H_
#define MAIN_H_



#include <sys/epoll.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/io.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/bitarray.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <conexiones.c>
#include <configuracion.h>
#include <serializador.h>
#include <semaphore.h>
#include <commons/log.h>
#include <pthread.h>

t_log* log;

void configuracion(char*, char*);
int32_t levantarConexion();






#endif
