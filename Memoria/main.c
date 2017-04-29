#include "main.h"
archivoConfigMemoria* t_archivoConfig;
t_config *config;
struct sockaddr_in direccionServidor;
int servidor;
int activado;
int cliente;
struct sockaddr_in direccionCliente;
unsigned int tamanoDireccion;
char* buffer;
int main(int argc, char**argv) {
	ConfiguracionMem(argv[1]);
	levantarConexion();
	return EXIT_SUCCESS;
}
void ConfiguracionMem(char *dir){
	t_archivoConfig = malloc(
				sizeof(archivoConfigMemoria));
	configuracionMemoria(t_archivoConfig, config, dir);
}
int levantarConexion(){
	llenarSocketAdrr(&direccionServidor,t_archivoConfig->PUERTO);

		servidor = socket(AF_INET, SOCK_STREAM, 0);

		activado = 1;
		setsockopt(servidor, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

		if (bind(servidor, (void*) &direccionServidor, sizeof(direccionServidor))
				!= 0) {
			perror("Falló el bind");
			return 1;
		}

		printf("Estoy escuchando\n");
		listen(servidor, 100);

		cliente = accept(servidor, (void*) &direccionCliente, &tamanoDireccion);
		printf("Recibí una conexión en %d!!\n", cliente);
		send(cliente, "hola, soy memoria", sizeof("hola, soy memoria"), 0);

		buffer = malloc(1000);

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
}

