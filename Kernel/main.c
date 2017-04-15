#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include <commons/config.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <conexiones.c>
#include <configuracion.h>

int main(int argc, char**argv) {
	struct sockaddr_in direccionCliente;
	unsigned int tamanoDireccion;
	int fdmax;
	int newfd;        // descriptor de socket de nueva conexión aceptada
	char buf[256];    // buffer para datos del cliente
	int nbytes;
	int i, j;
	fd_set master;   // conjunto maestro de descriptores de fichero
	fd_set read_fds; // conjunto temporal de descriptores de fichero para select()
	archivoConfigKernel* t_archivoConfig = malloc(sizeof(archivoConfigKernel));

	t_config *config = malloc(sizeof(t_config));
	printf("arranquemos, so\n");
	configuracionKernel(t_archivoConfig, config, argv[1]);

	FD_ZERO(&master);    // borra los conjuntos maestro y temporal
	FD_ZERO(&read_fds);

	struct sockaddr_in direccionMem;
	direccionMem.sin_family = AF_INET;
	direccionMem.sin_addr.s_addr = inet_addr(t_archivoConfig->IP_MEMORIA);
	direccionMem.sin_port = htons(t_archivoConfig->PUERTO_MEMORIA);

	int clienteMEM = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(clienteMEM, (void*) &direccionMem, sizeof(direccionMem)) != 0) {
		perror("No se pudo conectar");
		return 1;
	}
	send(clienteMEM, "hola, soy Kernel", sizeof("hola, soy Kernel"), 0);
	char* buffer = malloc(1000);
	int bytesRecibidos = recv(clienteMEM, buffer, 1000, 0);
	while (bytesRecibidos <= 0) {
		bytesRecibidos = recv(clienteMEM, buffer, 100, 0);
	}

	printf("me llego de memoria: %s\n", buffer);
	struct sockaddr_in direccionFs;
	direccionFs.sin_family = AF_INET;
	direccionFs.sin_addr.s_addr = inet_addr(t_archivoConfig->IP_FS);
	direccionFs.sin_port = htons(t_archivoConfig->PUERTO_FS);

	int clientefs = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(clientefs, (void*) &direccionFs, sizeof(direccionFs)) != 0) {
		perror("No se pudo conectar");
		return 1;
	}
	send(clientefs, "hola, soy Kernel", sizeof("hola, soy Kernel"), 0);
	char* bufferFs = malloc(1000);
	int bytesRecibidosFs = recv(clientefs, bufferFs, 1000, 0);
	while (bytesRecibidosFs <= 0) {
		bytesRecibidosFs = recv(clientefs, bufferFs, 100, 0);
	}
	printf("me llego de fs: %s\n", bufferFs);
	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = INADDR_ANY;
	direccionServidor.sin_port = htons(5000);
	memset(&(direccionServidor.sin_zero), '\0', 8);

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

	FD_SET(servidor, &master);
	// seguir la pista del descriptor de fichero mayor
	fdmax = servidor; // por ahora es éste
	for (;;) {
		read_fds = master; // cópialo
		if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
			perror("select");
			exit(1);
		}

		for (i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &read_fds)) { // ¡¡tenemos datos!!
				if (i == servidor) {
					// gestionar nuevas conexiones
					tamanoDireccion = sizeof(direccionCliente);
					if ((newfd = accept(servidor, (void*) &direccionCliente,
							&tamanoDireccion)) == -1) {
						perror("accept");
					} else {
						FD_SET(newfd, &master); // añadir al conjunto maestro
						if (newfd > fdmax) {    // actualizar el máximo
							fdmax = newfd;
						}
						printf("selectserver: new connection from %s on "
								"socket %d\n",
								inet_ntoa(direccionCliente.sin_addr), newfd);
					}
				} else {
					// gestionar datos de un cliente
					if ((nbytes = recv(i, buf, sizeof(buf), 0)) <= 0) {

						// error o conexión cerrada por el cliente
						if (nbytes == 0) {
							// conexión cerrada
							printf("selectserver: socket %d hung up\n", i);
						} else {
							perror("recv");
						}
						close(i); // bye!
						FD_CLR(i, &master); // eliminar del conjunto maestro
					} else {
						nbytes[buf] = '\0';
						printf("Me llegaron %d bytes con %s\n", nbytes, buf);
						if (buf[0] != 'h') {
							send(clientefs, buf, nbytes, 0);
							send(clienteMEM, buf, nbytes, 0);
							// tenemos datos de algún cliente
							for (j = 0; j <= fdmax; j++) {
								// ¡enviar a todo el mundo!
								if (FD_ISSET(j, &master)) {
									// excepto al listener y a nosotros mismos
									if (j != servidor && j != i) {
										if (send(j, buf, nbytes, 0) == -1) {
											perror("send");
										}
									}
								}
							}
						}

					}
				}
			}
		}
	}

	return EXIT_SUCCESS;
}
