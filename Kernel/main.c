#include "main.h"

struct sockaddr_in direccionCliente;
uint32_t tamanoDireccion;
script* unScript;
int32_t fdmax;
int32_t cpuDisponible;
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
int32_t MARCOS_SIZE;
int32_t activado;
int32_t clientefs;
int32_t bytesRecibidos;
int32_t tamanoPaquete;
int32_t processID = 0;
struct sockaddr_in direccionFs;
struct sockaddr_in direccionServidor;
t_queue* colaNew;
t_queue* colaCodigosAMemoria;
t_queue* colaReady;
t_queue* colaExec;
t_queue* colaBlock;
t_queue* colaExit;
t_queue* colaCpu;
sem_t gradoMultiprogramacion;
sem_t semUnScript;
sem_t semNew;
sem_t semReady;
sem_t semEnvioPaginas;
sem_t semProcesar;
sem_t semCpu;
sem_t semEntraElProceso;
sem_t semPaginaEnviada;
pthread_mutex_t mutexColaNew;
pthread_mutex_t mutexColaExit;
pthread_mutex_t mutexColaBlock;
pthread_mutex_t mutexColaEx;
pthread_mutex_t mutexColaReady;
pthread_mutex_t mutexProcesar;
pthread_mutex_t mutexProcesarPaquetes;
pthread_mutex_t mutexColaCpu;
pthread_t hiloPlanificadorLargoPlazo;
pthread_t hiloEnviarProceso;
pthread_t hiloPlanificadorCortoPlazo;
pthread_t hiloConexionFS;
pthread_t hiloConexionMemoria;
int noInteresa;
t_log * logger;

int32_t main(int argc, char**argv) {
	logger = log_create("KERNEL.log", "KERNEL", 0, LOG_LEVEL_INFO);

	configuracion(argv[1]);
	conectarConMemoria();
	ConectarConFS();
	pthread_create(&hiloPlanificadorLargoPlazo, NULL, planificadorLargoPlazo,
	NULL);
	pthread_create(&hiloPlanificadorCortoPlazo, NULL, planificadorCortoPlazo,
	NULL);

	pthread_create(&hiloEnviarProceso, NULL, procesarScript, NULL);
	levantarServidor();
	pthread_join(hiloPlanificadorLargoPlazo, NULL);
	pthread_join(hiloPlanificadorCortoPlazo, NULL);
	pthread_join(hiloEnviarProceso, NULL);
	return EXIT_SUCCESS;
}

void configuracion(char*dir) {
	t_archivoConfig = malloc(sizeof(archivoConfigKernel));
	configuracionKernel(t_archivoConfig, config, dir);
	sem_init(&semUnScript, 0, 0);
	sem_init(&semNew, 0, 0);
	sem_init(&semReady, 0, 0);
	sem_init(&semEntraElProceso, 0, 0);
	sem_init(&semPaginaEnviada, 0, 0);
	sem_init(&semEnvioPaginas, 0, 1);
	sem_init(&semCpu, 0, 0);
	sem_init(&semProcesar, 0, 1);
	//ojo con este semaforo
	sem_init(&gradoMultiprogramacion, 0, t_archivoConfig->GRADO_MULTIPROG);
	colaNew = queue_create();
	colaExec = queue_create();
	colaReady = queue_create();
	colaBlock = queue_create();
	colaExit = queue_create();
	colaCpu = queue_create();
	colaCodigosAMemoria = queue_create();
}
int32_t conectarConMemoria() {
	llenarSocketAdrrConIp(&direccionMem, t_archivoConfig->IP_MEMORIA,
			t_archivoConfig->PUERTO_MEMORIA);
	clienteMEM = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(clienteMEM, (void*) &direccionMem, sizeof(direccionMem)) != 0) {
		perror("No se pudo conectar con memoria\n");
		return 1;
	}
	Serializar(KERNEL, 4, &noInteresa, clienteMEM);
	paquete* paqueteRecibido = Deserializar(clienteMEM);
	if (paqueteRecibido->header < 0) {
		perror("Memoria se desconectó");
		return 1;
	}
	procesar(paqueteRecibido->package, paqueteRecibido->header,
			paqueteRecibido->size);

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
	Serializar(KERNEL, 4, &noInteresa, clientefs);
	paquete* paqueteRecibido = Deserializar(clientefs);
	if (paqueteRecibido->header < 0) {
		perror("Kernel se desconectó");
		return 1;
	}
	procesar(paqueteRecibido->package, paqueteRecibido->header,
			paqueteRecibido->size);
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
							int codigoOperacion = (int) (*codigoKernel) - 48;
							printf("Got data on stdin: %d\n", codigoOperacion);
							free(codigoKernel);
						} else {
							// fd closed
							perror("read()");
						}
					} else {
						// gestionar datos de un cliente
						paquete* paqueteRecibido = Deserializar(i);

						// error o conexión cerrada por el cliente
						if (paqueteRecibido->header == -1) {
							// conexión cerrada
							//PROCESAR: SI ES CONSOLA, ABORTAR PIDS
							//SI ES CPU, SCAARLA DE LA LISA
							close(i); // bye!
							FD_CLR(i, &master); // eliminar del conjunto maestro
							printf("selectserver: socket %d hung up\n", i);
						} else if (paqueteRecibido->header == -2) {
							close(i); // bye!
							FD_CLR(i, &master); // eliminar del conjunto maestro
							perror("recv");
						} else {

							procesar(paqueteRecibido->package,
									paqueteRecibido->header,
									paqueteRecibido->size, i);

						}
					}
				}
			}
		}
	}
}

void procesarScript() {
	while (1) {
		sem_wait(&semUnScript);
		int cantidadDePaginas = ceil(
				(double) unScript->tamano / (double) MARCOS_SIZE);
		int cantidadDePaginasToales = cantidadDePaginas
				+ t_archivoConfig->STACK_SIZE;
		Serializar(TAMANO, sizeof(int), &cantidadDePaginasToales, clienteMEM);
		sem_wait(&semEntraElProceso);
		if (header == OK) {
			proceso* unProceso = malloc(sizeof(proceso));
			programControlBlock* unPcb = malloc(sizeof(programControlBlock));
			unPcb->cantidadDePaginas = cantidadDePaginas;
			crearPCB(unScript->codigo, unPcb);
			unProceso->pcb = unPcb;
			unProceso->socketCONSOLA = unScript->socket;
			Serializar(PID, 4, &processID, unScript->socket);
			enviarProcesoAMemoria(unPcb->cantidadDePaginas, unScript->codigo,
					unScript->tamano);
			sem_wait(&semEnvioPaginas);
			pthread_mutex_lock(&mutexColaNew);
			queue_push(colaNew, unProceso);
			pthread_mutex_unlock(&mutexColaNew);
			sem_post(&semNew);
			free(unScript->codigo);
		}
	}

}

void procesar(char * paquete, int32_t id, int32_t tamanoPaquete, int32_t socket) {
	switch (id) {
	case ARCHIVO: {
		//printf("%s\n", paquete);
		//printf("%d\n", tamanoPaquete);
		//paquete[tamanoPaquete] = '\0';
		unScript = malloc(sizeof(script));
		unScript->tamano = tamanoPaquete;
		unScript->codigo = malloc(tamanoPaquete);
		unScript->socket = socket;
		memcpy(unScript->codigo, paquete, tamanoPaquete);
		sem_post(&semUnScript);
		break;
	}
	case FILESYSTEM: {
		printf("Se conecto FS\n");
		break;
	}

	case CPU: {
		pthread_mutex_lock(&mutexColaCpu);
		queue_push(colaCpu, socket);
		pthread_mutex_unlock(&mutexColaCpu);
		sem_post(&semCpu);
		printf("Se conecto CPU\n");
		break;
	}
	case CONSOLA: {
		printf("Se conecto Consola\n");
		break;
	}
	case MEMORIA: {
		memcpy(&MARCOS_SIZE, (paquete), sizeof(int));
		printf("Se conecto memoria\n");
		break;
	}
	case CODIGO: {
		break;
	}
	case OK: {
		break;
	}
	case PAGINAENVIADA: {
		sem_post(&semPaginaEnviada);
		break;
	}
	case ENTRAPROCESO: {
		sem_post(&semEntraElProceso);
		memcpy(&header, paquete, 4);

		printf("header %d\n", header);
		break;
	}
	case PROGRAMATERMINADO: {
		pthread_mutex_lock(&mutexColaEx);
		proceso * procesoTerminado = sacarProcesoDeEjecucion(socket);
		pthread_mutex_unlock(&mutexColaEx);
		pthread_mutex_lock(&mutexColaExit);
		queue_push(colaExit, procesoTerminado);
		pthread_mutex_unlock(&mutexColaExit);
		pthread_mutex_lock(&mutexColaCpu);
		queue_push(colaCpu, socket);
		pthread_mutex_unlock(&mutexColaCpu);
		sem_post(&semCpu);
		break;
	}
		//procesar pid muerto
		//semaforear los procesar
	}
	return;
}

void crearPCB(char* codigo, programControlBlock *unPcb) {
	t_metadata_program* metadata_program;
	metadata_program = metadata_desde_literal(codigo);
	processID++;
	unPcb->programId = processID;
	unPcb->exitCode = -1;
	unPcb->programCounter = 0;
	unPcb->tamanoIndiceCodigo = (metadata_program->instrucciones_size);
	unPcb->indiceCodigo = malloc(unPcb->tamanoIndiceCodigo * 2 * sizeof(int));

	for (i = 0; i < metadata_program->instrucciones_size; i++) {
		/*printf("Instruccion inicio:%d offset:%d %.*s",
		 metadata_program->instrucciones_serializado[i].start,
		 metadata_program->instrucciones_serializado[i].offset,
		 metadata_program->instrucciones_serializado[i].offset,
		 codigo + metadata_program->instrucciones_serializado[i].start);*/
		unPcb->indiceCodigo[i * 2] =
				metadata_program->instrucciones_serializado[i].start;
		unPcb->indiceCodigo[i * 2 + 1] =
				metadata_program->instrucciones_serializado[i].offset;
	}
	unPcb->tamanoindiceEtiquetas = metadata_program->etiquetas_size;
	unPcb->indiceEtiquetas = malloc(
			unPcb->tamanoindiceEtiquetas * sizeof(char));
	memcpy(unPcb->indiceEtiquetas, metadata_program->etiquetas,
			unPcb->tamanoindiceEtiquetas * sizeof(char));
	unPcb->indiceStack = list_create();

	indiceDeStack *indiceInicial;
	indiceInicial = malloc(sizeof(indiceDeStack));
	indiceInicial->args = list_create();
	indiceInicial->vars = list_create();
	indiceInicial->tamanoArgs = 0;
	indiceInicial->tamanoVars = 0;
	indiceInicial->pos = 0;
	list_add(unPcb->indiceStack, (void*) indiceInicial);
	unPcb->tamanoIndiceStack = 1;
	metadata_destruir(metadata_program);
}

void enviarProcesoAMemoria(int cantidadDePaginas, char* codigo,
		int tamanoPaquete) {
	int offset = 0;
	int i = 0;
	int sobra = tamanoPaquete % MARCOS_SIZE;
	for (i = 1; i <= cantidadDePaginas; i++) {
		if (i == cantidadDePaginas) {
			void* envioPagina = malloc(MARCOS_SIZE + sizeof(int));
			char * sobras = "0000000000000000000000";
			memcpy(envioPagina, codigo + offset, sobra);
			memcpy(envioPagina + sobra, sobras, MARCOS_SIZE - sobra);
			memcpy(envioPagina + MARCOS_SIZE, &processID, sizeof(processID));
			Serializar(PAGINA, MARCOS_SIZE + sizeof(int), envioPagina,
					clienteMEM);
			printf("envio pagina %s\n", envioPagina);
			sem_wait(&semPaginaEnviada);
			free(envioPagina);
		} else {
			void* envioPagina = malloc(MARCOS_SIZE + sizeof(int));
			memcpy(envioPagina, codigo + offset, MARCOS_SIZE);
			memcpy(envioPagina + MARCOS_SIZE, &processID, sizeof(processID));
			offset = offset + MARCOS_SIZE;
			Serializar(PAGINA, MARCOS_SIZE + sizeof(int), envioPagina,
					clienteMEM);
			printf("envio pagina %s\n", envioPagina);
			sem_wait(&semPaginaEnviada);
			free(envioPagina);
		}

	}
	sem_post(&semEnvioPaginas);
}

void planificadorLargoPlazo() {
	proceso* unProcesoPlp;

	while (1) {
		sem_wait(&semNew);
		pthread_mutex_lock(&mutexColaNew);
		unProcesoPlp = queue_pop(colaNew);
		pthread_mutex_unlock(&mutexColaNew);
		pthread_mutex_lock(&mutexColaReady);
		queue_push(colaReady, unProcesoPlp);
		pthread_mutex_unlock(&mutexColaReady);
		sem_post(&semReady);
	}

}

void planificadorCortoPlazo() {
	proceso* unProcesoPcp;
	int socket;
	while (1) {
		sem_wait(&semReady);
		sem_wait(&semCpu);
		pthread_mutex_lock(&mutexColaReady);
		unProcesoPcp = queue_pop(colaReady);
		pthread_mutex_unlock(&mutexColaReady);
		pthread_mutex_lock(&mutexColaCpu);
		socket = (int) queue_pop(colaCpu);
		pthread_mutex_unlock(&mutexColaCpu);
		ejecutar(unProcesoPcp, socket);

	}

}

void ejecutar(proceso* procesoAEjecutar, int socket) {
	procesoAEjecutar->socketCPU = socket;
	pthread_mutex_lock(&mutexColaEx);
	queue_push(colaExec, procesoAEjecutar);
	pthread_mutex_unlock(&mutexColaEx);
	serializarPCB(procesoAEjecutar->pcb, socket);
}

proceso* sacarProcesoDeEjecucion(int sock) {
	int a = 0, t;
	proceso *procesoABuscar;
	while (procesoABuscar = (proceso*) list_get(colaExec->elements, a)) {
		if (procesoABuscar->socketCPU == sock)
			return (proceso*) list_remove(colaExec->elements, a);
		a++;
	}
	printf("NO HAY PROCESO\n");
	exit(0);
	return NULL;
}
