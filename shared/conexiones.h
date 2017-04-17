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

void llenarSocketAdrr(struct sockaddr_in*, int );
void llenarSocketAdrrConIp(struct sockaddr_in*, char*, int );

#endif
