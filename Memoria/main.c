#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include <commons/config.h>
#include <sys/socket.h>
#include <arpa/inet.h>




int main(int argc, char**argv){
	char *ipMem = "127.0.0.2";
		int puertoMem = 5002;
			//configuracion(t_archivoConfig, config, argv[1]);
		struct sockaddr_in direccionServidor;
		direccionServidor.sin_family = AF_INET;
		direccionServidor.sin_addr.s_addr = inet_addr(ipMem);
		direccionServidor.sin_port = htons(puertoMem);

			int servidor = socket(AF_INET, SOCK_STREAM, 0);

			int activado = 1;
			setsockopt(servidor, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

			if (bind(servidor, (void*) &direccionServidor, sizeof(direccionServidor)) != 0) {
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

//void configuracion(archivoConfigKernel *unArchivo, t_config* config, char dir[]){


//}