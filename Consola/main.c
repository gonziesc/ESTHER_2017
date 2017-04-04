#include "main.h"

archivoConfigConsola* t_archivoConfig;
t_config *config;
struct sockaddr_in direccionKernel;
int32_t cliente;

int32_t main(int argc, char**argv) {
	Configuracion(argv[1]);
	ConectarseConKernel();
	return EXIT_SUCCESS;
}
void Configuracion(char* dir) {
	t_archivoConfig = malloc(sizeof(archivoConfigConsola));
	configuracionConsola(t_archivoConfig, config, dir);
}
int32_t ConectarseConKernel() {
	llenarSocketAdrrConIp(&direccionKernel, t_archivoConfig->IP_KERNEL,
			t_archivoConfig->PUERTO_KERNEL);
	cliente = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(cliente, (void*) &direccionKernel, sizeof(direccionKernel))
			!= 0) {
		perror("No se pudo conectar");
		return 1;
	}
	send(cliente, "hola, soy consola", sizeof("hola, soy consola"), 0);
	while (1) {
		char mensaje[1000];
		scanf("%s", mensaje);
		if (strlen(mensaje) > 100) {
			printf("mensaje muy largo");
			return 1;
		}
		send(cliente, mensaje, strlen(mensaje), 0);
	}
}
