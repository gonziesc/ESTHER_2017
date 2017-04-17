#include "main.h"

int main(int argc, char**argv) {
	archivoConfigMemoria* t_archivoConfig = malloc(
			sizeof(archivoConfigMemoria));
	t_config *config = malloc(sizeof(t_config));
	printf("memoria \n");
	configuracionMemoria(t_archivoConfig, config, argv[1]);
	struct sockaddr_in direccionServidor;
	llenarSocketAdrr(&direccionServidor,t_archivoConfig->PUERTO);

	int servidor = socket(AF_INET, SOCK_STREAM, 0);

	int activado = 1;
	setsockopt(servidor, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

	if (bind(servidor, (void*) &direccionServidor, sizeof(direccionServidor))
			!= 0) {
		perror("Falló el bind");
		return 1;
	}

	printf("Estoy escuchando\n");
	listen(servidor, 100);

	//------------------------------

	struct sockaddr_in direccionCliente;
	unsigned int tamanoDireccion;
	int cliente = accept(servidor, (void*) &direccionCliente, &tamanoDireccion);
	printf("Recibí una conexión en %d!!\n", cliente);
	send(cliente, "hola, soy memoria", sizeof("hola, soy memoria"), 0);

	//------------------------------

	char* buffer = malloc(1000);

	while (1) {
		int bytesRecibidos = recv(cliente, buffer, 1000, 0);
		if (bytesRecibidos <= 0) {
			perror("El CLIENTE se desconectó");
			return 1;
		}

		buffer[bytesRecibidos] = '\0';
		printf("Me llegaron %d bytes con %s\n", bytesRecibidos, buffer);
	}

	free(buffer);

	return EXIT_SUCCESS;
}

