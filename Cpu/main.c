#include "main.h"

archivoConfigCPU* t_archivoConfig;
t_config *config;
struct sockaddr_in direccionKernel;
int32_t cliente;
char* buffer;
struct sockaddr_in direccionMem;
int32_t clienteMEM;
int32_t bytesRecibidos;

int32_t main(int argc, char**argv) {
	Configuracion(argv[1]);
	ConectarConKernel();
	//conectarConMemoria();
	return EXIT_SUCCESS;
}
void Configuracion(char* dir){
	t_archivoConfig = malloc(sizeof(archivoConfigCPU));
	configuracionCpu(t_archivoConfig, config, dir);
}

int32_t conectarConMemoria(){
	llenarSocketAdrrConIp(&direccionMem,t_archivoConfig->IP_MEMORIA,
					t_archivoConfig->PUERTO_MEMORIA);
		clienteMEM = socket(AF_INET, SOCK_STREAM, 0);
		if (connect(clienteMEM, (void*) &direccionMem, sizeof(direccionMem))
				!= 0) {
			perror("No se pudo conectar");
			return 1;
		}
		send(clienteMEM, "hola, soy cpu", sizeof("hola, soy cpu"), 0);

		buffer = malloc(1000);

		while (1) {
			int32_t bytesRecibidos = recv(clienteMEM, buffer, 1000, 0);
			if (bytesRecibidos <= 0) {
				perror("El chabón se desconectó o bla.");
				return 1;
			}

			buffer[bytesRecibidos] = '\0';
			printf("Me llegaron %d bytes con %s\n", bytesRecibidos, buffer);
		}

		free(buffer);
}

int32_t ConectarConKernel(){
	llenarSocketAdrrConIp(&direccionKernel,t_archivoConfig->IP_KERNEL,
					t_archivoConfig->PUERTO_KERNEL);

		cliente = socket(AF_INET, SOCK_STREAM, 0);
		if (connect(cliente, (void*) &direccionKernel, sizeof(direccionKernel))
				!= 0) {
			perror("No se pudo conectar");
			return 1;
		}
		//Serializar(0, 4, 0, cliente);
		//buffer = malloc(1000);

		while (1) {
		//	int32_t bytesRecibidos = recv(cliente, buffer, 1000, 0);
		//	if (bytesRecibidos <= 0) {
		//		perror("El chabón se desconectó o bla.");
		//		return 1;
		//	}

		//	buffer[bytesRecibidos] = '\0';
		//	printf("Me llegaron %d bytes con %s\n", bytesRecibidos, buffer);
		}

		free(buffer);
}

