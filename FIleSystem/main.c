#include "main.h"
archivoConfigFS* t_archivoConfig;
t_config *config;
struct sockaddr_in direccionServidor;
int servidor;
int activado;
struct sockaddr_in direccionCliente;
unsigned int tamanoDireccion;
int cliente;
char* buffer;
int main(int argc, char**argv) {
	configuracion(argv[1]);
	levantarConexion();
	return EXIT_SUCCESS;
}
void configuracion(char * dir){
	t_archivoConfig = malloc(sizeof(archivoConfigFS));
	configuracionFS(t_archivoConfig, config, dir);
}
int levantarConexion(){
	llenarSocketAdrr(&direccionServidor,t_archivoConfig->PUERTO_KERNEL);
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
		send(cliente, "hola, soy fs", sizeof("hola, soy fs"), 0);
		printf("Recibí una conexión en %d!!\n", cliente);
		buffer = malloc(1000);
		while (1) {
			int bytesRecibidos = recv(cliente, buffer, 1000, 0);
			if (bytesRecibidos <= 0) {
				perror("Kernel se desconectó");
				return 1;
			}

			buffer[bytesRecibidos] = '\0';
			printf("Me llegaron %d bytes con %s\n", bytesRecibidos, buffer);
		}
		free(buffer);
}
