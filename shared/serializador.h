#ifndef MAIN_H_
#define MAIN_H_

#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <commons/config.h>

void Serializar(int32_t id, int32_t tamanioArchivo, void* buffer,int32_t  socket);
void Deserializar(void* buffer,int32_t  socket, void* archivoDesempaquetado);



#endif
