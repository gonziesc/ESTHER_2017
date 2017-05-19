#include "main.h"

struct sockaddr_in direccionCliente;
uint32_t tamanoDireccion;
int32_t fdmax;
int32_t newfd;        // descriptor de socket de nueva conexión aceptada
int32_t header;    // buffer para datos del cliente
int32_t nbytes;
int32_t i, j;
fd_set master;   // conjunto maestro de descriptores de fichero
fd_set read_fds; // conjunto temporal de descriptores de fichero para select()
archivoConfigKernel* t_archivoConfig;
t_config *config;
struct sockaddr_in direccionMem;
int32_t clienteMEM;
char* buffer;
char buf[3];    // buffer para datos del cliente
int32_t servidor;
int32_t activado;
int32_t clientefs;
int32_t bytesRecibidos;
int32_t tamanoPaquete;
int32_t PID = 0;
struct sockaddr_in direccionFs;
struct sockaddr_in direccionServidor;

int32_t main(int argc, char**argv) {
	configuracion(argv[1]);
	conectarConMemoria();
	ConectarConFS();
	levantarServidor();
	return EXIT_SUCCESS;
}

void configuracion(char*dir) {
	t_archivoConfig = malloc(sizeof(archivoConfigKernel));
	configuracionKernel(t_archivoConfig, config, dir);
}
int32_t conectarConMemoria() {
	llenarSocketAdrrConIp(&direccionMem, t_archivoConfig->IP_MEMORIA,
			t_archivoConfig->PUERTO_MEMORIA);
	clienteMEM = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(clienteMEM, (void*) &direccionMem, sizeof(direccionMem)) != 0) {
		perror("No se pudo conectar con memoria\n");
		return 1;
	}
	Serializar(5, 4, 0, clienteMEM);
	bytesRecibidos = recv(clienteMEM, &header, 4, 0);
	while (bytesRecibidos <= 0) {
		bytesRecibidos = recv(clienteMEM, &header, 4, 0);
		printf("%d", header);
	}

	char* paquete = Deserializar(header, clienteMEM, &tamanoPaquete);
	procesar(paquete, header, tamanoPaquete);
	return 0;
}
int32_t ConectarConFS() {
	llenarSocketAdrrConIp(&direccionFs, t_archivoConfig->IP_FS,
			t_archivoConfig->PUERTO_FS);
	clientefs = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(clientefs, (void*) &direccionFs, sizeof(direccionFs)) != 0) {
		perror("No se pudo conectar con fs\n");
		return 1;
	}
	Serializar(5, 4, 0, clientefs);
	bytesRecibidos = recv(clientefs, &header, 4, 0);
	while (bytesRecibidos <= 0) {
		bytesRecibidos = recv(clientefs, &header, 4, 0);
		printf("%d", header);
	}

	Deserializar(header, clientefs);
	char* paquete = Deserializar(header, clientefs, &tamanoPaquete);
	procesar(paquete, header, tamanoPaquete);
	return 0;
}
int32_t levantarServidor() {
	FD_ZERO(&master);    // borra los conjuntos maestro y temporal
	FD_ZERO(&read_fds);
	llenarSocketAdrr(&direccionServidor, 5000);
	servidor = socket(AF_INET, SOCK_STREAM, 0);
	activado = 1;
	setsockopt(servidor, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));
	if (bind(servidor, (void*) &direccionServidor, sizeof(direccionServidor))
			!= 0) {
		perror("Falló el bind\n");
		return 1;
	}
	printf("Estoy escuchando\n");
	listen(servidor, 100);
	FD_SET(0, &master);
	FD_SET(clienteMEM, &master);
	FD_SET(clientefs, &master);
	FD_SET(servidor, &master);
	// seguir la pista del descriptor de fichero mayor
	fdmax = servidor; // por ahora es éste
	while (1) {
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
						perror("accept\n");
					} else {
						printf("numer serv%d\n", newfd);
						FD_SET(newfd, &master); // añadir al conjunto maestro
						if (newfd > fdmax) {    // actualizar el máximo
							fdmax = newfd;
						}
						printf("selectserver: new connection from %s on "
								"socket %d\n",
								inet_ntoa(direccionCliente.sin_addr), newfd);
					}
				} else {
					if (i == 0) {
						char * codigoKernel = malloc(4);
						int logitudIO = read(0, codigoKernel, 4);
						if (logitudIO > 0) {
							int codigoOperacion = (int) (*codigoKernel) -48;
							printf("Got data on stdin: %d\n", codigoOperacion);
							free(codigoKernel);
						} else {
							// fd closed
							perror("read()");
						}
					} else {
						// gestionar datos de un cliente
						if ((nbytes = recv(i, &header, sizeof(header), 0))
								<= 0) {

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
							char* paquete = Deserializar(header, i,
									&tamanoPaquete);
							procesar(paquete, header, tamanoPaquete, i);

						}
					}
				}
			}
		}
	}
}

	void procesar(char * paquete, int32_t id, int32_t tamanoPaquete,
			int32_t socket) {
		switch (id) {
		case ARCHIVO: {
			printf("%s\n", paquete);
			/*
			 Serializar(ARCHIVO, tamanoPaquete, paquete, clienteMEM);
			 recv(clienteMEM, &header, 4, 0);
			 if (header == 0) {
			 programControlBlock *unPcb = malloc(sizeof(programControlBlock));
			 crearPCB(paquete, unPcb);
			 Serializar(PID, 4, sizeof(int32_t), socket);
			 } */
			int cantidadDePaginas = tamanoPaquete / 25;
			char * send = &cantidadDePaginas;
			// 25 representa marcos size, agregar al config de kernel
			Serializar(TAMANO, 4, send, clienteMEM);
			break;
		}
		case FILESYSTEM: {
			printf("Se conecto FS\n");
			break;
		}
		case KERNEL: {
			printf("Se conecto Kernel\n");
			break;
		}
		case CPU: {
			printf("Se conecto CPU\n");
			break;
		}
		case CONSOLA: {
			printf("Se conecto Consola\n");
			break;
		}
		case MEMORIA: {
			printf("Se conecto memoria\n");
			break;
		}
		case CODIGO: {

		}
		case OK: {
			printf("noo\n");
		}
		}
	}

	void crearPCB(char* codigo, programControlBlock *unPcb) {
		unPcb->programId = PID;
		PID++;
		unPcb->programCounter = 0;
	}
