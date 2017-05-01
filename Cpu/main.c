#include "main.h"

archivoConfigCPU* t_archivoConfig;
t_config *config;
struct sockaddr_in direccionKernel;
int32_t cliente;
char* buffer;
struct sockaddr_in direccionMem;
int32_t clienteMEM;
int32_t bytesRecibidos;
int32_t header;

int32_t main(int argc, char**argv) {
	Configuracion(argv[1]);
	ConectarConKernel();
	//conectarConMemoria();
	return EXIT_SUCCESS;
}
void Configuracion(char* dir) {
	t_archivoConfig = malloc(sizeof(archivoConfigCPU));
	configuracionCpu(t_archivoConfig, config, dir);
}

int32_t conectarConMemoria() {
	llenarSocketAdrrConIp(&direccionMem, t_archivoConfig->IP_MEMORIA,
			t_archivoConfig->PUERTO_MEMORIA);
	clienteMEM = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(clienteMEM, (void*) &direccionMem, sizeof(direccionMem)) != 0) {
		perror("No se pudo conectar");
		return 1;
	}
	Serializar(6, 4, 0, clienteMEM);

	while (1) {
		int32_t bytesRecibidos = recv(clienteMEM, &header, 4, 0);
		if (bytesRecibidos <= 0) {
			perror("El chabón se desconectó o bla.");
			return 1;
		}
		Deserializar(header, 0);
	}
}

int32_t ConectarConKernel() {
	llenarSocketAdrrConIp(&direccionKernel, t_archivoConfig->IP_KERNEL,
			t_archivoConfig->PUERTO_KERNEL);

	cliente = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(cliente, (void*) &direccionKernel, sizeof(direccionKernel))
			!= 0) {
		perror("No se pudo conectar");
		return 1;
	}
	//Serializar(6, 4, 0, cliente);
	//buffer = malloc(1000);

	while (1) {
		int32_t bytesRecibidos = recv(cliente, &header, 4, 0);
		if (bytesRecibidos <= 0) {
			perror("Kernel se desconectó");
			return 1;
		}
		//Deserializar(header, 0);
	}

	free(buffer);
}

