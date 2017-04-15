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

int main(int argc, char**argv) {

	archivoConfigCPU* t_archivoConfig = malloc(sizeof(archivoConfigCPU));
	t_config *config = malloc(sizeof(t_config));
	printf("cpu \n");
	configuracionCpu(t_archivoConfig, config, argv[1]);

	struct sockaddr_in direccionKernel;
	direccionKernel.sin_family = AF_INET;
	direccionKernel.sin_addr.s_addr = inet_addr(t_archivoConfig->IP_KERNEL);
	direccionKernel.sin_port = htons(t_archivoConfig->PUERTO_KERNEL);

	int cliente = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(cliente, (void*) &direccionKernel, sizeof(direccionKernel))
			!= 0) {
		perror("No se pudo conectar");
		return 1;
	}
	send(cliente, "hola, soy cpu", sizeof("hola, soy cpu"), 0);

	char* buffer = malloc(1000);

	while (1) {
		int bytesRecibidos = recv(cliente, buffer, 1000, 0);
		if (bytesRecibidos <= 0) {
			perror("El chabón se desconectó o bla.");
			return 1;
		}

		buffer[bytesRecibidos] = '\0';
		printf("Me llegaron %d bytes con %s\n", bytesRecibidos, buffer);
	}

	free(buffer);

	return EXIT_SUCCESS;
}

