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
		perror("No se pudo conectar con memoria");
		return 1;
	}
	send(clienteMEM, "hola, soy Kernel", sizeof("hola, soy Kernel"), 0);
	buffer = malloc(1000);
	bytesRecibidos = recv(clienteMEM, buffer, 1000, 0);
	while (bytesRecibidos <= 0) {
		bytesRecibidos = recv(clienteMEM, buffer, 100, 0);
	}

	printf("me llego de memoria: %s\n", buffer);
	return 0;
}
int32_t ConectarConFS() {
	llenarSocketAdrrConIp(&direccionFs, t_archivoConfig->IP_FS,
			t_archivoConfig->PUERTO_FS);
	clientefs = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(clientefs, (void*) &direccionFs, sizeof(direccionFs)) != 0) {
		perror("No se pudo conectar con fs");
		return 1;
	}
	send(clientefs, "hola, soy Kernel", sizeof("hola, soy Kernel"), 0);
	char* bufferFs = malloc(1000);
	int32_t bytesRecibidosFs = recv(clientefs, bufferFs, 1000, 0);
	while (bytesRecibidosFs <= 0) {
		bytesRecibidosFs = recv(clientefs, bufferFs, 100, 0);
	}
	printf("me llego de fs: %s\n", bufferFs);
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
		perror("Falló el bind");
		return 1;
	}
	printf("Estoy escuchando\n");
	listen(servidor, 100);

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
					if ((nbytes = recv(i, &header, sizeof(header), 0)) <= 0) {

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
						printf("Me llegaron %d bytes con %d\n", nbytes, header);
						char* paquete = Deserializar(header, i);
						printf("llego el paquete %s\n", paquete);
						/*send(clientefs, buffer, sizeof(buffer), 0);
						send(clienteMEM, buffer, sizeof(buffer), 0);
						//tenemos datos de algún cliente
						for (j = 0; j <= fdmax; j++) {
							// ¡enviar a todo el mundo!
							if (FD_ISSET(j, &master)) {
								if (j != servidor && j != i) {
									if (send(j, buffer, sizeof(buffer), 0) == -1) {
										perror("send");
									}
								}
							}
						}*/

					}
				}
			}
		}
	}
}

void procesar() {

}

void crearPCB(char buffer[]) {
	PCB *unPcb = malloc(sizeof(PCB));
	unPcb->programId = PID;
	PID++;
	unPcb->programCounter = 0;
	int cantidadDePaginas = 5;


}
