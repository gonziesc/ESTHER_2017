#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include <commons/config.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <tads.c>
#include <conexiones.c>
#include <configuracion.h>



int main(int argc, char**argv){
	archivoConfigConsola* t_archivoConfig = malloc(sizeof(archivoConfigConsola));
	t_config *config = malloc(sizeof(t_config));
	configuracionConsola(t_archivoConfig, config, argv[1]);


	struct sockaddr_in direccionKernel;
	direccionKernel.sin_family = AF_INET;
	direccionKernel.sin_addr.s_addr = inet_addr(t_archivoConfig->IP_KERNEL);
	direccionKernel.sin_port = htons(t_archivoConfig->PUERTO_KERNEL);

		int cliente = socket(AF_INET, SOCK_STREAM, 0);
		if (connect(cliente, (void*) &direccionKernel, sizeof(direccionKernel)) != 0) {
			perror("No se pudo conectar");
			return 1;
		}

		while (1) {
			char mensaje[1000];

			scanf("%s", mensaje);
			if(strlen(mensaje) > 100){
				printf("mensaje muy largo");
				return 1;
			}
			send(cliente, mensaje, strlen(mensaje), 0);
		}





		return EXIT_SUCCESS;
}


