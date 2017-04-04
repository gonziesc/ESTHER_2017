#include "conexiones.h"


void llenarSocketAdrr(struct sockaddr_in* direccionServidor, int puerto){

		direccionServidor->sin_family = AF_INET;
		direccionServidor->sin_addr.s_addr = INADDR_ANY;
		direccionServidor->sin_port = htons(puerto);

}

void llenarSocketAdrrConIp(struct sockaddr_in* direccionServidor,char*ip, int puerto){

		direccionServidor->sin_family = AF_INET;
		direccionServidor->sin_addr.s_addr = inet_addr(ip);
		direccionServidor->sin_port = htons(puerto);

}
