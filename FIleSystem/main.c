#include "main.h"

int main(int argc, char**argv) {
	archivoConfigFS* t_archivoConfig = malloc(sizeof(archivoConfigFS));
	t_config *config = malloc(sizeof(t_config));
	configuracionFS(t_archivoConfig, config, argv[1]);
	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = INADDR_ANY;
	direccionServidor.sin_port = htons(t_archivoConfig->PUERTO_KERNEL);

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
	send(cliente, "hola, soy fs", sizeof("hola, soy fs"), 0);
	printf("Recibí una conexión en %d!!\n", cliente);

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

