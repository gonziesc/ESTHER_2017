#include "main.h"

archivoConfigCPU* t_archivoConfig;
t_config *config;
struct sockaddr_in direccionKernel;
int cliente;
char* buffer;
int main(int argc, char**argv) {
	Configuracion(argv[1]);
	ConectarConKernel();
	return EXIT_SUCCESS;
}
void Configuracion(char* dir){
	t_archivoConfig = malloc(sizeof(archivoConfigCPU));
	configuracionCpu(t_archivoConfig, config, dir);
}
int ConectarConKernel(){
	llenarSocketAdrrConIp(&direccionKernel,t_archivoConfig->IP_KERNEL,
					t_archivoConfig->PUERTO_KERNEL);

		cliente = socket(AF_INET, SOCK_STREAM, 0);
		if (connect(cliente, (void*) &direccionKernel, sizeof(direccionKernel))
				!= 0) {
			perror("No se pudo conectar");
			return 1;
		}
		send(cliente, "hola, soy cpu", sizeof("hola, soy cpu"), 0);

		buffer = malloc(1000);

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
}
