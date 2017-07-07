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
char * metaDataLeida;
struct sockaddr_in direccionMem;
int32_t clienteMEM;
char* buffer;
char* punteroPaginaHeap;
char* nuevaPaginaHeap;
char buf[3];    // buffer para datos del cliente
int32_t servidor;
int32_t MARCOS_SIZE;
int32_t activado;
int32_t clientefs;
int32_t ultimaPaginaPid[100];
int32_t bytesRecibidos;
int32_t tamanoPaquete;
int32_t processID = 0;
struct sockaddr_in direccionFs;
struct sockaddr_in direccionServidor;
t_queue* colaHeap;
t_queue* colaCapaFs;
t_queue* colaLiberaHeap;
t_queue* colaNew;
t_queue* colaCodigosAMemoria;
t_queue* colaReady;
t_queue* colaExec;
t_queue* colaExit;
t_queue* colaCpu;
t_queue* colaProcesosConsola;
t_queue* colaProcesosBloqueados;
t_queue** colas_semaforos;
t_list* listaAdmHeap;
sem_t gradoMultiprogramacion;
sem_t semUnScript;
sem_t semNew;
sem_t semReady;
sem_t semEnvioPaginas;
sem_t semProcesar;
sem_t semCpu;
sem_t semEntraElProceso;
sem_t semPaginaEnviada;
sem_t semPunteroPaginaHeap;
sem_t semMemoriaReservoHeap;
sem_t semMetaDataLeida;
sem_t semProcesoAHeap;
sem_t semLiberarHeap;
sem_t semCapaFs;
sem_t semExisteArchivo;
sem_t semCrearArchivo;
sem_t semBorrarArchivo;
sem_t semObtenerDatos;
sem_t semGuardarDatos;
sem_t semTerminoDataHeap;
int validarExisteArchivo;
int validarCrearArchivo;
int validarBorrarArchivo;
int validarObtenerDatos;
char * datosDeFs;
int validarGuardarDatos;

pthread_mutex_t mutexDatosDeFs;
pthread_mutex_t mutexColaNew;
pthread_mutex_t mutexColaExit;
pthread_mutex_t mutexColaEx;
pthread_mutex_t mutexColaReady;
pthread_mutex_t mutexProcesar;
pthread_mutex_t mutexProcesarPaquetes;
pthread_mutex_t mutexColaCpu;
pthread_mutex_t mutexProcesarScript;
pthread_mutex_t mutexConfig;
pthread_mutex_t mutexProcesosBloqueados;
pthread_mutex_t mutexMemoria;
pthread_mutex_t mutexListaAdminHeap;
pthread_mutex_t mutexColaHeap;
pthread_mutex_t mutexColaLiberaHeap;
pthread_mutex_t mutexColaCapaFs;
pthread_t hiloPlanificadorLargoPlazo;
pthread_t hiloEnviarProceso;
pthread_t hiloPlanificadorCortoPlazo;
pthread_t hiloConexionFS;
pthread_t hiloConexionMemoria;
pthread_t hiloProcesarHeap;
pthread_t hiloLiberaHepa;
pthread_t hiloCapaFs;
int noInteresa;
t_log * logger;
datosHeap tablaHeap[100];
int indiceLibreHeap = 0;
t_list* tablaArchivosGlobal;
t_list* listaTablasProcesos;

int32_t main(int argc, char**argv) {
	logger = log_create("KERNEL.log", "KERNEL", 0, LOG_LEVEL_INFO);

	configuracion(argv[1]);
	conectarConMemoria();
	ConectarConFS();
	pthread_create(&hiloPlanificadorLargoPlazo, NULL, planificadorLargoPlazo,
	NULL);
	pthread_create(&hiloPlanificadorCortoPlazo, NULL, planificadorCortoPlazo,
	NULL);
	pthread_create(&hiloProcesarHeap, NULL, procesarHeap, NULL);
	pthread_create(&hiloEnviarProceso, NULL, procesarScript, NULL);
	pthread_create(&hiloLiberaHepa, NULL, liberarHeap, NULL);
	pthread_create(&hiloCapaFs, NULL, procesarCapaFs, NULL);
	levantarServidor();
	pthread_join(hiloProcesarHeap, NULL);
	pthread_join(hiloCapaFs, NULL);
	pthread_join(hiloPlanificadorLargoPlazo, NULL);
	pthread_join(hiloPlanificadorCortoPlazo, NULL);
	pthread_join(hiloEnviarProceso, NULL);
	pthread_join(hiloLiberaHepa, NULL);
	return EXIT_SUCCESS;
}

void configuracion(char*dir) {
	t_archivoConfig = malloc(sizeof(archivoConfigKernel));
	configuracionKernel(t_archivoConfig, config, dir);
	sem_init(&semCapaFs, 0, 0);
	sem_init(&semUnScript, 0, 0);
	sem_init(&semNew, 0, 0);
	sem_init(&semReady, 0, 0);
	sem_init(&semEntraElProceso, 0, 0);
	sem_init(&semPaginaEnviada, 0, 0);
	sem_init(&semEnvioPaginas, 0, 1);
	sem_init(&semCpu, 0, 0);
	sem_init(&semPunteroPaginaHeap, 0, 0);
	sem_init(&semProcesar, 0, 1);
	sem_init(&semMemoriaReservoHeap, 0, 0);
	sem_init(&semMetaDataLeida, 0, 0);
	sem_init(&semProcesoAHeap, 0, 0);
	sem_init(&semLiberarHeap, 0, 0);
	sem_init(&semTerminoDataHeap, 0, 0);
	//ojo con este semaforo
	sem_init(&gradoMultiprogramacion, 0, t_archivoConfig->GRADO_MULTIPROG);
	colaNew = queue_create();
	colaExec = queue_create();
	colaReady = queue_create();
	colaExit = queue_create();
	colaCpu = queue_create();
	colaHeap = queue_create();
	colaCapaFs = queue_create();
	listaAdmHeap = list_create();
	colaProcesosConsola = queue_create();
	colaCodigosAMemoria = queue_create();
	colaProcesosBloqueados = queue_create();
	colaLiberaHeap = queue_create();
	tablaArchivosGlobal = list_create();
	listaTablasProcesos = list_create();
	colas_semaforos = malloc(
			strlen((char*) t_archivoConfig->SEM_INIT) * sizeof(char*));

	for (i = 0; i < strlen((char*) t_archivoConfig->SEM_INIT) / sizeof(char*);
			i++) {

		colas_semaforos[i] = malloc(sizeof(t_queue*));
		colas_semaforos[i] = queue_create();
	}
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
			paqueteRecibido->size);    //TODO: FREE PAQUETE
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
							procesarEntrada(codigoOperacion);
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

void procesarEntrada(int codigoOperacion) {
	switch (codigoOperacion) {
	case 1: {

		break;
	}
	case 2: {
		int pidAMatar;
		printf("Ingrese pid a finalizar\n");
		scanf("%d", &pidAMatar);
		//TODO avisar a consola
		abortarProgramaPorConsola(pidAMatar, -35);
		//TODO cambiar codigo
		break;
	}
	case 3: {

		break;
	}
	case 4: {

		break;
	}
	}
}

void procesarScript() {
	while (1) {
		sem_wait(&semUnScript);
		pthread_mutex_lock(&mutexProcesarScript);
		int cantidadDePaginas = ceil(
				(double) unScript->tamano / (double) MARCOS_SIZE);
		int cantidadDePaginasToales = cantidadDePaginas
				+ t_archivoConfig->STACK_SIZE + 1;

		pthread_mutex_lock(&mutexMemoria);
		Serializar(TAMANO, sizeof(int), &cantidadDePaginasToales, clienteMEM);
		pthread_mutex_unlock(&mutexMemoria);
		sem_wait(&semEntraElProceso);
		if (header == OK) {
			proceso* unProceso = malloc(sizeof(proceso));
			programControlBlock* unPcb = malloc(sizeof(programControlBlock));
			procesoConsola* unaConsola = malloc(sizeof(procesoConsola));

			unPcb->cantidadDePaginas = cantidadDePaginas;
			crearPCB(unScript->codigo, unPcb);
			unaConsola->pid = processID;
			unaConsola->consola = unScript->socket;
			ultimaPaginaPid[processID] = cantidadDePaginasToales;
			//TODO semaforo?
			queue_push(colaProcesosConsola, unaConsola);
			unProceso->pcb = unPcb;
			unProceso->socketCONSOLA = unScript->socket;
			char * enviocantidadDePaginas = malloc(2 * sizeof(int));
			memcpy(enviocantidadDePaginas, &processID, sizeof(int));
			memcpy(enviocantidadDePaginas + 4, &cantidadDePaginasToales,
					sizeof(int));
			pthread_mutex_lock(&mutexMemoria);
			Serializar(INICIALIZARPROCESO, 8, enviocantidadDePaginas,
					clienteMEM);
			free(enviocantidadDePaginas);
			pthread_mutex_unlock(&mutexMemoria);
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
		pthread_mutex_unlock(&mutexProcesarScript);
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
	case MEMORIARESERVOHEAP: {
		sem_post(&semMemoriaReservoHeap);
		break;
	}
	case METADATALEIDA: {
		metaDataLeida = malloc(tamanoPaquete);
		memcpy(metaDataLeida, paquete, tamanoPaquete);
		sem_post(&semMetaDataLeida);
		break;
	}

	case CPU: {
		datosKernelACpu enviar;
		enviar.quantum = t_archivoConfig->QUANTUM;
		enviar.quantumSleep = t_archivoConfig->QUANTUM_SLEEP;
		enviar.stack = t_archivoConfig->STACK_SIZE;
		printf("%s\n", t_archivoConfig->ALGORITMO);
		if (!(strcmp(t_archivoConfig->ALGORITMO, "RR"))) {
			enviar.algoritmo = 1;
		} else {
			enviar.algoritmo = 0;
		}
		Serializar(DATOSKERNELCPU, sizeof(datosKernelACpu), &enviar, socket);
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
		//TODO liberar paginas procesos memoria
		Serializar(PROGRAMATERMINADO, 4, &procesoTerminado->pcb->programId,
				procesoTerminado->socketCONSOLA);
		sem_post(&semCpu);
		break;
	}
	case PUNTEROPAGINAHEAP:
	case ESCRITURAPAGINA: {
		punteroPaginaHeap = malloc(tamanoPaquete);
		memcpy(punteroPaginaHeap, paquete, tamanoPaquete);
		sem_post(&semPunteroPaginaHeap);
		break;
	}
	case MATARPIDPORCONSOLA: {
		int pid;
		char* fechaFIn = temporal_get_string_time();
		memcpy(&pid, paquete, sizeof(int));
		abortarProgramaPorConsola(pid, codeFinalizarPrograma);
		break;
	}
	case NOENTROPROCESO: {
		//TODO abortar
		break;
	}
	case ABORTOSTACKOVERFLOW: {
		programControlBlock* pcbRecibido = deserializarPCB(paquete);
		proceso* unProceso = sacarProcesoDeEjecucionPorPid(
				pcbRecibido->programId);
		destruirPCB(unProceso->pcb);
		unProceso->pcb = pcbRecibido;
		abortar(unProceso, codeStackOverflow);
		break;
	}
		//procesar pid muerto
		//semaforear los procesar
	case DESCONECTARCONSOLA: {
		abortarTodosLosProgramasDeConsola(socket);
		break;
	}
	case FINDEQUATUM: {
		programControlBlock* pcbRecibido = deserializarPCB(paquete);
		proceso* unProceso = sacarProcesoDeEjecucionPorPid(
				pcbRecibido->programId);
		destruirPCB(unProceso->pcb);
		unProceso->pcb = pcbRecibido;
		pthread_mutex_lock(&mutexColaReady);
		queue_push(colaReady, unProceso);
		pthread_mutex_unlock(&mutexColaReady);
		sem_post(&semReady);
		pthread_mutex_lock(&mutexColaCpu);
		queue_push(colaCpu, socket);
		pthread_mutex_unlock(&mutexColaCpu);
		sem_post(&semCpu);
		break;
	}
	case ASIGNOVALORVARIABLECOMPARTIDA: {
		proceso* unProceso;
		int valor;
		memcpy(&valor, paquete, 4);
		char * variable = malloc(tamanoPaquete - 4);
		memcpy(variable, paquete + 4, tamanoPaquete);
		pthread_mutex_lock(&mutexColaEx);
		unProceso = sacarProcesoDeEjecucion(socket);
		queue_push(colaExec, unProceso);
		pthread_mutex_unlock(&mutexColaEx);
		pthread_mutex_lock(&mutexConfig);
		escribeVariable(variable, valor);
		pthread_mutex_unlock(&mutexConfig);
		break;
	}

	case VALORVARIABLECOMPARTIDA: {
		proceso* unProceso;
		char * variable = malloc(tamanoPaquete);
		memcpy(variable, paquete, tamanoPaquete);
		pthread_mutex_lock(&mutexColaEx);
		unProceso = sacarProcesoDeEjecucion(socket);
		queue_push(colaExec, unProceso);
		pthread_mutex_unlock(&mutexColaEx);
		pthread_mutex_lock(&mutexConfig);
		int envio = pideVariable(variable);
		Serializar(VALORVARIABLECOMPARTIDA, sizeof(int), &envio, socket);
		pthread_mutex_unlock(&mutexConfig);
		free(variable);
		break;
	}

	case IMPRIMIRPROCESO: {
		proceso* unProceso;
		int tamanioAEnviar;
		int tamanioTexto = tamanoPaquete - 4;
		char *impresion = malloc(tamanioTexto + 4);
		memcpy(impresion, paquete, tamanioTexto);
		pthread_mutex_lock(&mutexColaEx);
		unProceso = sacarProcesoDeEjecucion(socket);
		queue_push(colaExec, unProceso);
		pthread_mutex_unlock(&mutexColaEx);
		memcpy(impresion + tamanioTexto, &(unProceso->pcb->programId), 4);
		Serializar(IMPRESIONPORCONSOLA, tamanioTexto + 4, impresion,
				unProceso->socketCONSOLA);
		free(impresion);
		break;
	}
	case PROCESOWAIT: {
		char* semaforo = malloc(tamanoPaquete);
		memcpy(semaforo, paquete, tamanoPaquete);
		proceso* unProceso;
		pthread_mutex_lock(&mutexColaEx);
		unProceso = sacarProcesoDeEjecucion(socket);
		pthread_mutex_unlock(&mutexColaEx);
		pthread_mutex_lock(&mutexConfig);
		int valorSemaforo = pideSemaforo(semaforo);
		int mandar;
		if (valorSemaforo <= 0) {
			mandar = 1;
			Serializar(PROCESOWAIT, sizeof(int), &mandar, socket);
			procesoBloqueado *unProcesoBloqueado = malloc(
					sizeof(procesoBloqueado));
			unProcesoBloqueado->pid = unProceso->pcb->programId;
			unProcesoBloqueado->semaforo = semaforo;
			pthread_mutex_lock(&mutexProcesosBloqueados);
			queue_push(colaProcesosBloqueados, unProcesoBloqueado);
			pthread_mutex_unlock(&mutexProcesosBloqueados);
			pthread_mutex_lock(&mutexColaEx);
			queue_push(colaExec, unProceso);
			pthread_mutex_unlock(&mutexColaEx);

		} else {
			escribeSemaforo(semaforo, pideSemaforo(semaforo) - 1);
			mandar = 0;
			Serializar(PROCESOWAIT, sizeof(int), &mandar, socket);
			pthread_mutex_lock(&mutexColaEx);
			queue_push(colaExec, unProceso);
			pthread_mutex_unlock(&mutexColaEx);
		}

		pthread_mutex_unlock(&mutexConfig);
		//free(semaforo);
		break;
	}
	case PROCESOSIGNAL: {
		proceso* unProceso;
		char* semaforo = malloc(tamanoPaquete);
		memcpy(semaforo, paquete, tamanoPaquete);
		pthread_mutex_lock(&mutexColaEx);
		unProceso = sacarProcesoDeEjecucion(socket);
		queue_push(colaExec, unProceso);
		pthread_mutex_unlock(&mutexColaEx);
		pthread_mutex_lock(&mutexConfig);
		liberaSemaforo(semaforo);
		pthread_mutex_unlock(&mutexConfig);
		free(semaforo);
		break;
	}
	case SEBLOQUEOELPROCESO: {
		programControlBlock* pcbRecibido = deserializarPCB(paquete);
		proceso* unProceso = sacarProcesoDeEjecucionPorPid(
				pcbRecibido->programId);
		destruirPCB(unProceso->pcb);
		char* semaforo = conseguirSemaforoDeBloqueado(pcbRecibido->programId);
		unProceso->pcb = pcbRecibido;
		bloqueoSemaforo(unProceso, semaforo);
		queue_push(colaCpu, socket);
		sem_post(&semCpu);
		break;

	}
	case PROCESOPIDEHEAP: {
		proceso* unProceso;
		int pid, cantidad;
		memcpy(&pid, paquete, 4);
		memcpy(&cantidad, paquete + 4, 4);
		pthread_mutex_lock(&mutexColaEx);
		unProceso = sacarProcesoDeEjecucion(socket);
		queue_push(colaExec, unProceso);
		pthread_mutex_unlock(&mutexColaEx);
		hiloHeap* procesoAHeap = malloc(sizeof(hiloHeap));
		procesoAHeap->pid = pid;
		procesoAHeap->socket = socket;
		procesoAHeap->cantidad = cantidad;
		pthread_mutex_lock(&mutexColaHeap);
		queue_push(colaHeap, procesoAHeap);
		pthread_mutex_unlock(&mutexColaHeap);
		sem_post(&semProcesoAHeap);

		break;
	}
	case PROCESOLIBERAHEAP: {
		proceso* unProceso;
		pthread_mutex_lock(&mutexColaEx);
		unProceso = sacarProcesoDeEjecucion(socket);
		queue_push(colaExec, unProceso);
		pthread_mutex_unlock(&mutexColaEx);
		int pid, pagina, offsetPagina;
		memcpy(&pid, paquete, 4);
		memcpy(&pagina, paquete + 4, 4);
		memcpy(&offsetPagina, paquete + 8, 4);
		liberaDatosHeap* procesoAHeap = malloc(sizeof(liberaDatosHeap));
		procesoAHeap->pid = pid;
		procesoAHeap->socket = socket;
		procesoAHeap->offset = offsetPagina;
		procesoAHeap->pagina = pagina;
		pthread_mutex_lock(&mutexColaLiberaHeap);
		queue_push(colaLiberaHeap, procesoAHeap);
		pthread_mutex_unlock(&mutexColaLiberaHeap);
		sem_post(&semLiberarHeap);
		break;
	}
	case ABRIRARCHIVO: {
		proceso* unProceso;
		pthread_mutex_lock(&mutexColaEx);
		unProceso = sacarProcesoDeEjecucion(socket);
		queue_push(colaExec, unProceso); //todo OJO CON TODOS ESTOS, PUEDE ABORTAR...
		pthread_mutex_unlock(&mutexColaEx);
		procesoACapaFs* p = malloc(sizeof(procesoACapaFs));
		p->codigoOperacion = 0;
		p->socket = socket;
		p->pid = unProceso->pcb->programId;
		int tamanoDireccion;
		int tamanoFlags;
		memcpy(&tamanoDireccion, paquete, 4);
		p->path = malloc(tamanoDireccion + 1);
		p->tamano = tamanoDireccion;
		memcpy(&tamanoFlags, paquete + 4, 4);
		p->permisos = malloc(tamanoFlags);
		memcpy(p->permisos, paquete + 8, tamanoFlags);
		memcpy(p->path, paquete + 8 + tamanoFlags, tamanoDireccion);
		char* barra_cero = "\0";
		memcpy(p->path + tamanoDireccion, barra_cero, 1);
		pthread_mutex_lock(&mutexColaCapaFs);
		queue_push(colaCapaFs, p);
		pthread_mutex_unlock(&mutexColaCapaFs);
		sem_post(&semCapaFs);
		break;
	}
	case BORRARARCHIVO: {
		proceso* unProceso;
		pthread_mutex_lock(&mutexColaEx);
		unProceso = sacarProcesoDeEjecucion(socket);
		queue_push(colaExec, unProceso); //todo OJO CON TODOS ESTOS, PUEDE ABORTAR...
		pthread_mutex_unlock(&mutexColaEx);
		procesoACapaFs *p = malloc(sizeof(procesoACapaFs));
		p->codigoOperacion = 1;
		p->socket = socket;
		p->pid = unProceso->pcb->programId;
		memcpy(&p->fd, paquete, 4);
		pthread_mutex_lock(&mutexColaCapaFs);
		queue_push(colaCapaFs, p);
		pthread_mutex_unlock(&mutexColaCapaFs);
		sem_post(&semCapaFs);
		break;
	}
	case CERRARARCHIVO: {
		proceso* unProceso;
		pthread_mutex_lock(&mutexColaEx);
		unProceso = sacarProcesoDeEjecucion(socket);
		queue_push(colaExec, unProceso); //todo OJO CON TODOS ESTOS, PUEDE ABORTAR...
		pthread_mutex_unlock(&mutexColaEx);
		procesoACapaFs* p = malloc(sizeof(procesoACapaFs));
		p->codigoOperacion = 2;
		p->socket = socket;
		p->pid = unProceso->pcb->programId;
		memcpy(&p->fd, paquete, 4);
		pthread_mutex_lock(&mutexColaCapaFs);
		queue_push(colaCapaFs, p);
		pthread_mutex_unlock(&mutexColaCapaFs);
		sem_post(&semCapaFs);
		break;
	}
	case ESCRIBIRARCHIVO: {
		proceso* unProceso;
		pthread_mutex_lock(&mutexColaEx);
		unProceso = sacarProcesoDeEjecucion(socket);
		queue_push(colaExec, unProceso); //todo OJO CON TODOS ESTOS, PUEDE ABORTAR...
		pthread_mutex_unlock(&mutexColaEx);
		procesoACapaFs* p = malloc(sizeof(procesoACapaFs));
		p->codigoOperacion = 3;
		p->socket = socket;
		p->pid = unProceso->pcb->programId;
		int tamanioTexto = tamanoPaquete - 4;
		p->data = malloc(tamanioTexto);
		memcpy(p->data, paquete, tamanioTexto);
		memcpy(&p->fd, paquete + tamanioTexto, 4); //TODO revisar esto
		pthread_mutex_lock(&mutexColaCapaFs);
		queue_push(colaCapaFs, p);
		pthread_mutex_unlock(&mutexColaCapaFs);
		sem_post(&semCapaFs);
		break;
	}
	case LEERARCHIVO: {
		proceso* unProceso;
		pthread_mutex_lock(&mutexColaEx);
		unProceso = sacarProcesoDeEjecucion(socket);
		queue_push(colaExec, unProceso); //todo OJO CON TODOS ESTOS, PUEDE ABORTAR...
		pthread_mutex_unlock(&mutexColaEx);
		procesoACapaFs* p = malloc(sizeof(procesoACapaFs));
		p->codigoOperacion = 4;
		p->socket = socket;
		p->pid = unProceso->pcb->programId;
		memcpy(&p->fd, paquete, 4);
		memcpy(&p->tamano, paquete + 4, 4);
		pthread_mutex_lock(&mutexColaCapaFs);
		queue_push(colaCapaFs, p);
		pthread_mutex_unlock(&mutexColaCapaFs);
		sem_post(&semCapaFs);
		break;
	}
	case MOVERCURSOR: {
		proceso* unProceso;
		pthread_mutex_lock(&mutexColaEx);
		unProceso = sacarProcesoDeEjecucion(socket);
		queue_push(colaExec, unProceso); //todo OJO CON TODOS ESTOS, PUEDE ABORTAR...
		pthread_mutex_unlock(&mutexColaEx);
		procesoACapaFs* p = malloc(sizeof(procesoACapaFs));
		p->codigoOperacion = 5;
		p->socket = socket;
		p->pid = unProceso->pcb->programId;
		memcpy(&p->fd, paquete, 4);
		memcpy(&p->posicion, paquete + 4, 4);
		pthread_mutex_lock(&mutexColaCapaFs);
		queue_push(colaCapaFs, p);
		pthread_mutex_unlock(&mutexColaCapaFs);
		sem_post(&semCapaFs);
		break;
	}
	case VALIDARARCHIVO: {
		memcpy(&validarExisteArchivo, paquete, 4);
		sem_post(&semExisteArchivo);
		break;
	}
	case BORRARARCHIVOFS: {
		memcpy(&validarBorrarArchivo, paquete, 4);
		sem_post(&semBorrarArchivo);
		break;
	}
	case CREARARCHIVO: {
		memcpy(&validarCrearArchivo, paquete, 4);
		sem_post(&semCrearArchivo);
		break;
	}
	case GUARDARDATOS: {
		memcpy(&validarGuardarDatos, paquete, 4);
		sem_post(&semGuardarDatos);
		break;
	}
	case OBTENERDATOS: {
		int tamano;
		memcpy(&tamano, paquete, 4);
		if (tamano == -1) {
			validarObtenerDatos = 0;
		} else {
			pthread_mutex_lock(&mutexDatosDeFs);
			datosDeFs = malloc(tamano);
			memcpy(datosDeFs, paquete, tamano);
			pthread_mutex_unlock(&mutexDatosDeFs);
		}
		sem_post(&semGuardarDatos);
		break;
	}

		return;
	}
}

void procesarCapaFs() {
	procesoACapaFs* unProceso;

	while (1) {
		sem_wait(&semCapaFs);
		pthread_mutex_lock(&mutexColaCapaFs);
		unProceso = queue_pop(colaCapaFs);
		pthread_mutex_unlock(&mutexColaCapaFs);
		switch (unProceso->codigoOperacion) {
		case 0: {
			abrirArchivo(unProceso);
			break;
		}
		case 1: {
			borrarArchivo(unProceso);
			break;
		}
		case 2: {
			cerrarArchivo(unProceso);
			break;
		}
		case 3: {
			escribirArchivo(unProceso);
			break;
		}
		case 4: {
			leerArchivo(unProceso);
			break;
		}
		case 5: {
			moverCursorArchivo(unProceso);
			break;
		}
		}

	}

}

int actualizarTablaDelProceso(int pid, char* flags, int indiceEnTablaGlobal) {
	int tablaProcesoExiste;

	_Bool verificaPid(indiceTablaProceso* entrada) {
		return entrada->pid == pid;
	}

	if (list_any_satisfy(listaTablasProcesos, (void*) verificaPid))
		tablaProcesoExiste = 1;
	else
		tablaProcesoExiste = 0;

	if (!tablaProcesoExiste) {
		printf("La tabla no existe\n");
		indiceTablaProceso* entradaNuevaTabla = malloc(
				sizeof(indiceTablaProceso));
		entradaNuevaTabla->pid = pid;
		entradaNuevaTabla->tablaProceso = list_create();

		entradaTablaProceso* entrada = malloc(sizeof(entradaTablaProceso));
		entrada->fd = 3;
		entrada->flags = flags;
		entrada->globalFd = indiceEnTablaGlobal;
		entrada->puntero = 0;

		list_add(entradaNuevaTabla->tablaProceso, entrada);

		list_add(listaTablasProcesos, entradaNuevaTabla);
		return entrada->fd;
	} else {
		printf("La tabla ya existe\n");

		indiceTablaProceso* entradaTablaExistente = list_remove_by_condition(
				listaTablasProcesos, (void*) verificaPid); //la remuevo para actualizarlo
		entradaTablaProceso* entrada = malloc(sizeof(entradaTablaProceso));
		entrada->fd = entradaTablaExistente->tablaProceso->elements_count + 3;
		entrada->flags = flags;
		entrada->globalFd = indiceEnTablaGlobal;
		printf("Agrego el indice :%d\n", entrada->globalFd);
		list_add(entradaTablaExistente->tablaProceso, entrada);
		list_add(listaTablasProcesos, entradaTablaExistente); //la vuelvo a agregar a la lista

		return entrada->fd;
	}
}

int borrarEntradaTablaProceso(int pid, int fd) {
	int globalFd;
	_Bool verificaPid(indiceTablaProceso* entrada) {
		return entrada->pid == pid;
	}
	_Bool verificaFd(entradaTablaProceso* entrada) {
		return entrada->fd == fd;
	}
	indiceTablaProceso* entradaTablaProceso2 = list_remove_by_condition(
			listaTablasProcesos, (void*) verificaPid);
	entradaTablaProceso* entrada2;
	entrada2 = list_remove_by_condition(entradaTablaProceso2->tablaProceso,
			(void*) verificaFd);
	globalFd = entrada2->globalFd;
	free(entrada2);
	list_add(listaTablasProcesos, entradaTablaProceso2);
	return globalFd;
}

int agregarEntradaEnTablaGlobal(char* direccion, int tamanioDireccion) {
	entradaTablaGlobal* entrada = malloc(sizeof(entradaTablaGlobal));
	entrada->open = 0;
	entrada->path = malloc(tamanioDireccion);
	entrada->path = direccion;

	list_add(tablaArchivosGlobal, entrada);

	return tablaArchivosGlobal->elements_count - 1;
}

int verificarEntradaEnTablaGlobal(char* direccion) { /*TODO: Mutex tablaGlobal*/

	_Bool verificaDireccion(entradaTablaGlobal* entrada) {
		return !strcmp(entrada->path, direccion);
	}

	if (list_is_empty(tablaArchivosGlobal))
		return 0;
	printf("La tabla global no esta vacia\n");

	if (list_any_satisfy(tablaArchivosGlobal, (void*) verificaDireccion))
		return 1;
	return 0;

}

void actualizarIndicesGlobalesEnTablasProcesos(int indiceTablaGlobal) {
	int i;
	int j;
	indiceTablaProceso* indiceTabla;
	entradaTablaProceso* entrada;
	for (i = 0; i < listaTablasProcesos->elements_count; i++) {
		indiceTabla = list_get(listaTablasProcesos, i);

		for (j = 0; j < indiceTabla->tablaProceso->elements_count; j++) {
			entrada = list_get(indiceTabla->tablaProceso, j);
			if (entrada->globalFd > indiceTablaGlobal)
				entrada->globalFd--; /*TODO: No pregunto si es igual porque se supone que no existe mas en esa tabla. Corroborarlo*/
		}
	}
}

void aumentarOpenEnTablaGlobal(char* direccion) {/*TODO: Mutex tablaGlobal*/
	_Bool verificaDireccion(entradaTablaGlobal* entrada) {
		if (!strcmp(entrada->path, direccion))
			return 1;
		return 0;
	}

	entradaTablaGlobal* entrada = list_remove_by_condition(tablaArchivosGlobal,
			(void*) verificaDireccion);
	entrada->open++;
	list_add(tablaArchivosGlobal, entrada);
}

void disminuirOpenYVerificarExistenciaEntradaGlobal(int indiceTablaGlobal) {
	entradaTablaGlobal* entrada = list_get(tablaArchivosGlobal,
			indiceTablaGlobal);
	entrada->open--;

	if (entrada->open == 0) {
		list_remove(tablaArchivosGlobal, indiceTablaGlobal);
		actualizarIndicesGlobalesEnTablasProcesos(indiceTablaGlobal);
		free(entrada);
	}
}

int buscarIndiceEnTablaGlobal(char* direccion) {
	int i;
	int indice = 0;
	entradaTablaGlobal* entrada;

	for (i = 0; i < tablaArchivosGlobal->elements_count; i++) {
		entrada = list_get(tablaArchivosGlobal, i);
		if (!strcmp(entrada->path, direccion))
			indice = i;
	}

	return indice;
}

char* buscarDireccionEnTablaGlobal(int indice) {
	entradaTablaGlobal* entrada = list_get(tablaArchivosGlobal, indice);

	return entrada->path;
}

void inicializarTablaProceso(int pid) {
	indiceTablaProceso* indiceNuevaTabla = malloc(sizeof(indiceTablaProceso));
	indiceNuevaTabla->pid = pid;
	indiceNuevaTabla->tablaProceso = list_create();

	entradaTablaProceso* entrada = malloc(sizeof(entradaTablaProceso));
	entrada->fd = 0;
	entrada->flags = "rwc";
	entrada->globalFd = 0;
	entrada->puntero = 0;

	list_add(indiceNuevaTabla->tablaProceso, entrada);

	list_add(listaTablasProcesos, indiceNuevaTabla);
}

int validarArchivo(char* ruta) {
	int tamano = sizeof(char) * strlen(ruta);
	int validado;
	void * envio = malloc(tamano + 4);
	memcpy(envio, &tamano, 4);
	memcpy(envio + 4, ruta, tamano);
	Serializar(VALIDARARCHIVO, tamano + 4, envio, clientefs);
	sem_wait(&semExisteArchivo);
	return validarExisteArchivo;
}

int crearArchivo(int socket_aceptado, char* direccion) {

	//TODO OJO CON LA BARRA DE ATRAS DEL NOMBRE

	int tamanoNombre = sizeof(char) * strlen(direccion);
	void* envio = malloc(tamanoNombre + 4);
	memcpy(envio, &tamanoNombre, 4);
	memcpy(envio + 4, direccion, tamanoNombre);
	Serializar(CREARARCHIVO, tamanoNombre + 4, envio, clientefs);
	sem_wait(&semCrearArchivo);
	free(envio);
	if (validarCrearArchivo < 0)
		printf("mal el fs");

	return validarCrearArchivo;

}

void abrirArchivo(procesoACapaFs* unProceso) {
	int archivoExistente = validarArchivo(unProceso->path);
	int indiceEnTablaGlobal;
	int resultadoEjecucion;
	int tienePermisoCreacion = 0;
	char* permiso_creacion = "c";
	if (string_contains(unProceso->permisos, permiso_creacion)) {
		tienePermisoCreacion = 1;
	}

	if (!archivoExistente && !tienePermisoCreacion) { //El archivo no eexiste en FS y no tiene permisos para crear entonces no hace nada
		//TODO excepcionPermisosCrear(unProceso->socket, unProceso->pid);
		free(unProceso->path);
		free(unProceso->permisos);
		return;
	}

	if (archivoExistente) {
		int entradaGlobalExistente = verificarEntradaEnTablaGlobal(
				unProceso->path);

		if (!entradaGlobalExistente) {
			indiceEnTablaGlobal = agregarEntradaEnTablaGlobal(unProceso->path,
					tamanoDireccion); //almacenar el Global FD
		} else {
			indiceEnTablaGlobal = buscarIndiceEnTablaGlobal(unProceso->path);
		}
		printf("Indice devuelto:%d\n", indiceEnTablaGlobal);
	}

	if (!archivoExistente && tienePermisoCreacion) {

		resultadoEjecucion = crearArchivo(unProceso->socket, unProceso->path);
		if (resultadoEjecucion < 0) {
			//TOdo excepcionFileSystem(unProceso->socket, unProceso->paid);
			free(unProceso->path);
			free(unProceso->permisos);
			return;
		}
		indiceEnTablaGlobal = agregarEntradaEnTablaGlobal(unProceso->path,
				unProceso->tamano);
		printf("Indice devuelto:%d\n", indiceEnTablaGlobal);
	}

	aumentarOpenEnTablaGlobal(unProceso->path);
	unProceso->fd = actualizarTablaDelProceso(unProceso->pid,
			unProceso->permisos, indiceEnTablaGlobal);
	resultadoEjecucion = 1;
	void * envio = malloc(8);
	memcpy(envio, &resultadoEjecucion, 4);
	memcpy(envio + 4, &unProceso->fd, 4);
	Serializar(ABRIRARCHIVO, 8, envio, unProceso->socket);

	printf("File descriptor:%d\n", unProceso->fd);
	free(envio);
	free(unProceso);
}

void cerrarArchivo(procesoACapaFs* unProceso) {
	int resultadoEjecucion;
	_Bool verificaPid(indiceTablaProceso* entrada) {
		return entrada->pid == unProceso->pid;
	}
	_Bool verificaFd(entradaTablaProceso* entrada) {
		return entrada->fd == unProceso->fd;
	}

	//verificar que la tabla de ese pid exista
	int tablaProcesoExiste;
	if (list_any_satisfy(listaTablasProcesos, (void*) verificaPid))
		tablaProcesoExiste = 1;
	else
		tablaProcesoExiste = 0;

	if (!tablaProcesoExiste) {
		//TODO excepcionSinTablaArchivos(unProceso->socket, unProceso->pid);
		return;
	} else {

		int encontroFd;
		indiceTablaProceso* entradaTablaProceso = list_remove_by_condition(
				listaTablasProcesos, (void*) verificaPid);
		if (list_any_satisfy(entradaTablaProceso->tablaProceso,
				(void*) verificaFd))
			encontroFd = 1;
		else
			encontroFd = 0;
		list_add(listaTablasProcesos, entradaTablaProceso);

		if (!encontroFd) {
			//todo excepcionFileDescriptorNoAbierto(socket, pid);
			return;
		} else {

			int indiceTablaGlobal = borrarEntradaTablaProceso(unProceso->pid,
					unProceso->fd);
			disminuirOpenYVerificarExistenciaEntradaGlobal(indiceTablaGlobal);
		}
	}
	resultadoEjecucion = 1;
	Serializar(CERRARARCHIVO, 4, &resultadoEjecucion, unProceso->socket);
	free(unProceso);
}

void escribirArchivo(procesoACapaFs* unProceso) {
	_Bool verificaPid(indiceTablaProceso* entrada) {
		return entrada->pid == unProceso->pid;
	}

	_Bool verificaFd(entradaTablaProceso* entrada) {
		return entrada->fd == unProceso->fd;
	}

	int tablaProcesoExiste;
	if (list_any_satisfy(listaTablasProcesos, (void*) verificaPid))
		tablaProcesoExiste = 1;
	else
		tablaProcesoExiste = 0;

	int encontroFd;

	if (!tablaProcesoExiste) {
		//TODO excepcionSinTablaArchivos(socket, pid);
		//free(informacion);
		return;
	} else {

		indiceTablaProceso* entradaTablaProceso2 = list_remove_by_condition(
				listaTablasProcesos, (void*) verificaPid);
		if (list_any_satisfy(entradaTablaProceso2->tablaProceso,
				(void*) verificaFd))
			encontroFd = 1;
		else
			encontroFd = 0;

		if (!encontroFd) {
			//TODO	excepcionFileDescriptorNoAbierto(socket, pid);
			return;
		}

		if (encontroFd) {
			entradaTablaProceso* entrada = list_remove_by_condition(
					entradaTablaProceso2->tablaProceso, (void*) verificaFd);

			int tiene_permisoEscritura = 0;
			const char *permiso_escritura = "w";
			if (string_contains(entrada->flags, permiso_escritura)) {
				tiene_permisoEscritura = 1;
			}

			if (!tiene_permisoEscritura) {
				//todo excepcionPermisosEscritura(socket, pid);
				//free(informacion);
				return;
			}

			char* direccion = buscarDireccionEnTablaGlobal(entrada->globalFd);

			char** array_dir = string_n_split(direccion, 12, "/");
			char* nombreArchivo = array_dir[0];
			int tamanoNombre = sizeof(char) * strlen(nombreArchivo);

			printf("Tamano del nombre :%d\n", tamanoNombre);
			printf("Nombre del archivo: %s\n", nombreArchivo);
			printf("Puntero:%d\n", entrada->puntero);
			printf("Tamano a escribir :%d\n", unProceso->tamano);
			printf("Informacion a escribir:%s\n", unProceso->data);
			void*envio = malloc(12 + tamanoNombre + unProceso->tamano);
			memcpy(envio, &tamanoNombre, 4);
			memcpy(envio + 4, &entrada->puntero, 4);
			memcpy(envio + 8, &unProceso->tamano, 4);
			memcpy(envio + 12, unProceso->data, unProceso->tamano);
			memcpy(envio + 12 + unProceso->tamano, nombreArchivo, tamanoNombre);
			Serializar(GUARDARDATOS, 12 + tamanoNombre + unProceso->tamano,
					envio, clientefs);

			list_add(entradaTablaProceso2->tablaProceso, entrada);
			list_add(listaTablasProcesos, entradaTablaProceso2);
			sem_wait(&semGuardarDatos);
			free(envio);
		}

	}
	Serializar(ESCRIBIRARCHIVO, 4, &validarGuardarDatos, unProceso->socket);
	free(unProceso);
}

void leerArchivo(procesoACapaFs* unProceso) {
	int resultadoEjecucion;
	_Bool verificaPid(indiceTablaProceso* entrada) {
		return entrada->pid == unProceso->pid;
	}

	_Bool verificaFd(entradaTablaProceso* entrada) {
		return entrada->fd == unProceso->fd;
	}

	int tablaProcesoExiste;
	if (list_any_satisfy(listaTablasProcesos, (void*) verificaPid))
		tablaProcesoExiste = 1;
	else
		tablaProcesoExiste = 0;

	if (!tablaProcesoExiste) {
		//TODO excepcionSinTablaArchivos(socket, pid);
		return;
	} else {

		int encontroFd;
		indiceTablaProceso* entradaTablaProceso2 = list_remove_by_condition(
				listaTablasProcesos, (void*) verificaPid);
		if (list_any_satisfy(entradaTablaProceso2->tablaProceso,
				(void*) verificaFd))
			encontroFd = 1;
		else
			encontroFd = 0;

		if (!encontroFd) {
			//TODO excepcionFileDescriptorNoAbierto(socket, pid);
			return;
		} else {
			entradaTablaProceso* entrada = list_remove_by_condition(
					entradaTablaProceso2->tablaProceso, (void*) verificaFd);

			int tiene_permisoLectura = 0;
			const char *permiso_lectura = "r";
			if (string_contains(entrada->flags, permiso_lectura)) {
				tiene_permisoLectura = 1;
			}
			if (!tiene_permisoLectura) {
				//TODO excepcionPermisosLectura(socket, pid);
				return;
			}

			char* direccion = buscarDireccionEnTablaGlobal(entrada->globalFd);
			list_add(entradaTablaProceso2->tablaProceso, entrada);
			list_add(listaTablasProcesos, entradaTablaProceso2);

			char** array_dir = string_n_split(direccion, 12, "/");
			char* nombreArchivo = array_dir[0];
			int tamanoNombre = sizeof(char) * strlen(nombreArchivo);

			printf("Tamano del nombre del archivo:%d\n", tamanoNombre);
			printf("Nombre del archivo:%s\n", nombreArchivo);
			printf("Puntero :%d\n", entrada->puntero);
			printf("Tamano a leer :%d\n", unProceso->tamano);

			void* envio = malloc(12 + tamanoNombre);
			memcpy(envio, &tamanoNombre, 4);
			memcpy(envio + 4, &unProceso->tamano, 4);
			memcpy(envio + 8, &entrada->puntero, 4);
			memcpy(envio + 12, nombreArchivo, tamanoNombre);
			Serializar(OBTENERDATOS, 12 + unProceso->tamano, envio, clientefs);
			free(envio);
			sem_wait(&semObtenerDatos);
			if (validarObtenerDatos == 1) {
				void * envio2 = malloc(8 + unProceso->tamano);
				memcpy(envio2, &validarObtenerDatos, 4);
				memcpy(envio2 + 4, &unProceso->tamano, 4);
				memcpy(envio2 + 8, datosDeFs, unProceso->tamano);
				Serializar(LEERARCHIVO, 8 + unProceso->tamano, envio,
						unProceso->socket);
				free(envio2);
				free(unProceso);
			} else {
				Serializar(LEERARCHIVO, 4, &validarObtenerDatos,
						unProceso->socket);
				free(unProceso);
			}

		}

	}
}

void borrarArchivo(procesoACapaFs* unProceso) {
	int resultadoEjecucion;
	_Bool verificaPid(indiceTablaProceso* entrada) {
		return entrada->pid == unProceso->pid;
	}
	_Bool verificaFd(entradaTablaProceso* entrada) {
		return entrada->fd == unProceso->fd;
	}

	/*TODO: Necesito ir a buscar la ruta del archivo, para validarla*/
	/*La busco de la sigueinte forma. Voy a la tabla por proceso, indexo con el fd.
	 * De ahi saco el globalFd, y voy a la tabla global. Saco la direccion*/

	//verificar que la tabla de ese pid exista
	int tablaProcesoExiste;
	if (list_any_satisfy(listaTablasProcesos, (void*) verificaPid))
		tablaProcesoExiste = 1;
	else
		tablaProcesoExiste = 0;

	if (!tablaProcesoExiste) {
		//TODO excepcionSinTablaArchivos(unProceso->socket, unProceso->pid);
		return;
	} else {
		int encontroFd;
		indiceTablaProceso* entradaTablaProceso = list_remove_by_condition(
				listaTablasProcesos, (void*) verificaPid);
		if (list_any_satisfy(entradaTablaProceso->tablaProceso,
				(void*) verificaFd))
			encontroFd = 1;
		else
			encontroFd = 0;
		list_add(listaTablasProcesos, entradaTablaProceso);

		if (!encontroFd) { /*Si ese archivo no lo tiene abierto no lo puede borrar*/
			//TODO excepcionFileDescriptorNoAbierto(unProceso->socket, unProceso->pid);
			return;
		} else {

			/* Voy a la tabla global y borro la entrada, y saco el indice(GlobalFd) de donde lo borre.
			 * Aca habria que ir a cada tabla de los procesos, y borrar la entrada. Uso como key el globalFd
			 * Habria actualizar CADA puntero de la tabla de los procesos. Solo se actualizan las tablas que tengan
			 * a los archivos que estaban por debajo de ese indice en la tabla global. Es disminuir en uno a cada globalFd.
			 * Todo esto deberia ir despues que el FS borre al archivo
			 * */

			//hacer los sends para que el FS borre ese archivo y deje los bloques libres
			/*if (resultadoEjecucion < 0) {
			 excepcionFileSystem(socket, pid);
			 return;
			 }*/
			int indiceGlobalFd = borrarEntradaTablaProceso(unProceso->pid,
					unProceso->fd);
			disminuirOpenYVerificarExistenciaEntradaGlobal(indiceGlobalFd);
		}

	}
	Serializar(BORRARARCHIVO, 4, &resultadoEjecucion, unProceso->socket);
	free(unProceso);
}
void moverCursorArchivo(procesoACapaFs* unProceso) {
	_Bool verificaPid(indiceTablaProceso* entrada) {
		return entrada->pid == unProceso->pid;
	}

	_Bool verificaFd(entradaTablaProceso* entrada) {
		return entrada->fd == unProceso->fd;
	}

	//verificar que la tabla de ese pid exista
	int tablaProcesoExiste;
	if (list_any_satisfy(listaTablasProcesos, (void*) verificaPid))
		tablaProcesoExiste = 1;
	else
		tablaProcesoExiste = 0;

	if (!tablaProcesoExiste) {
		//TODO excepcionSinTablaArchivos(socket, pid);
		return;
	} else {
		int encontroFd;
		indiceTablaProceso* entradaTablaProceso2 = list_remove_by_condition(
				listaTablasProcesos, (void*) verificaPid);
		if (list_any_satisfy(entradaTablaProceso2->tablaProceso,
				(void*) verificaFd))
			encontroFd = 1;
		else
			encontroFd = 0;

		if (!encontroFd) {
			//TODO excepcionArchivoInexistente(socket, pid);
		} else {

			entradaTablaProceso* entrada = list_remove_by_condition(
					entradaTablaProceso2->tablaProceso, (void*) verificaFd);
			entrada->puntero = unProceso->posicion;
			list_add(entradaTablaProceso2->tablaProceso, entrada);
			list_add(listaTablasProcesos, entradaTablaProceso2);

			int todobien = 1;
			Serializar(MOVERCURSOR, 4, &todobien, unProceso->socket);
			free(unProceso);
		}
	}
}

char* conseguirSemaforoDeBloqueado(int pid) {
	char* semaforo;
	int a = 0, t;
	procesoBloqueado * unProceso;
	while (unProceso = (procesoBloqueado*) list_get(
			colaProcesosBloqueados->elements, a)) {
		if (unProceso->pid == pid)
			return unProceso->semaforo;
		// TODO: REVISAR ESOTOOOO
		a++;
	}
}

void sacarSemaforosDesbloqueados(char* semaforo) {
	int a = 0, t;
	procesoBloqueado * unProceso;
	while (unProceso = (procesoBloqueado*) list_get(
			colaProcesosBloqueados->elements, a)) {
		if (strcmp((char*) unProceso->semaforo, semaforo) == 0)
			list_remove(colaProcesosBloqueados->elements, a);
		// TODO: REVISAR ESOTOOOO
		a++;
	}
}

void abortarTodosLosProgramasDeConsola(int socket) {
	procesoConsola* unProceso;
	bool esMiSocket(void * entrada) {
		procesoConsola * unproceso = (procesoConsola *) entrada;
		return unproceso->consola == socket;
	}
	unProceso = (procesoConsola*) list_remove_by_condition(
			colaProcesosConsola->elements, esMiSocket);
	while (unProceso != NULL) {
		abortarProgramaPorConsola(unProceso->pid, codeDesconexionConsola);
		unProceso = (procesoConsola*) list_remove_by_condition(
				colaProcesosConsola->elements, esMiSocket);

	}
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
			int offsetaux = 0;
			void* envioPagina = malloc(MARCOS_SIZE + 4 * sizeof(int));
			char * sobras = "0000000000000000000000";
			memcpy(envioPagina, &processID, sizeof(processID));
			memcpy(envioPagina + 4, &i, sizeof(int));
			memcpy(envioPagina + 8, &MARCOS_SIZE, sizeof(int));
			memcpy(envioPagina + 12, &offsetaux, sizeof(int));
			memcpy(envioPagina + 16, codigo + offset, sobra);
			memcpy(envioPagina + sobra + 16, sobras, MARCOS_SIZE - sobra);
			pthread_mutex_lock(&mutexMemoria);
			Serializar(PAGINA, MARCOS_SIZE + 4 * sizeof(int), envioPagina,
					clienteMEM);
			pthread_mutex_unlock(&mutexMemoria);
			printf("num pagina %d\n", i);
			sem_wait(&semPaginaEnviada);
			free(envioPagina);
		} else {
			int offsetaux = 0;
			void* envioPagina = malloc(MARCOS_SIZE + 4 * sizeof(int));
			memcpy(envioPagina, &processID, sizeof(processID));
			memcpy(envioPagina + 4, &i, sizeof(int));
			memcpy(envioPagina + 8, &MARCOS_SIZE, sizeof(int));
			memcpy(envioPagina + 12, &offsetaux, sizeof(int));
			memcpy(envioPagina + 16, codigo + offset, MARCOS_SIZE);
			offset = offset + MARCOS_SIZE;
			pthread_mutex_lock(&mutexMemoria);
			Serializar(PAGINA, MARCOS_SIZE + 4 * sizeof(int), envioPagina,
					clienteMEM);
			pthread_mutex_unlock(&mutexMemoria);
			printf("num pagina %d\n", i);
			sem_wait(&semPaginaEnviada);
			free(envioPagina);
		}

	}
	sem_post(&semEnvioPaginas);
}

void procesarHeap() {
	hiloHeap* heap;

	while (1) {
		sem_wait(&semProcesoAHeap);
		pthread_mutex_lock(&mutexColaHeap);
		heap = queue_pop(colaHeap);
		pthread_mutex_unlock(&mutexColaHeap);
		datosHeap *data = procesoPideHeap(heap->pid, heap->cantidad);
		sem_wait(&semTerminoDataHeap);
		void * envio = malloc(8);
		memcpy(envio, &data->pagina, 4);
		memcpy(envio + 4, &data->offset, 4);
		Serializar(PROCESOPIDEHEAP, 8, envio, heap->socket);
		free(heap);
		free(envio);
		free(data);
	}

}

void liberarHeap() {
	liberaDatosHeap* heap;

	while (1) {
		sem_wait(&semLiberarHeap);
		pthread_mutex_lock(&mutexColaLiberaHeap);
		heap = queue_pop(colaLiberaHeap);
		pthread_mutex_unlock(&mutexColaLiberaHeap);
		procesoLiberaHeap(heap->pid, heap->pagina, heap->offset);
		Serializar(PROCESOTERMINALIBERAHEAP, 4, &noInteresa, heap->socket);
		free(heap);
	}

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
	serializarPCB(procesoAEjecutar->pcb, socket, PCB);
	//el problema esta aca
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

proceso* sacarProcesoDeEjecucionPorPid(int pid) {
	int a = 0, t;
	proceso *procesoABuscar;
	while (procesoABuscar = (proceso*) list_get(colaExec->elements, a)) {
		if (procesoABuscar->pcb->programId == pid)
			return (proceso*) list_remove(colaExec->elements, a);
		a++;
	}
	printf("NO HAY PROCESO\n");
	exit(0);
	return NULL;
}

proceso* buscarProcesoEnEjecucion(int pid) {
	int a = 0, t;
	proceso *procesoABuscar;
	while (procesoABuscar = (proceso*) list_get(colaExec->elements, a)) {
		if (procesoABuscar->pcb->programId == pid)
			return procesoABuscar;
		a++;
	}
	printf("NO HAY PROCESO\n");
	exit(0);
	return NULL;
}

void modificarCantidadDePaginas(int pid) {
	int a = 0, t;
	proceso *procesoABuscar;
	while (procesoABuscar = (proceso*) list_get(colaExec->elements, a)) {
		if (procesoABuscar->pcb->programId == pid) {
			procesoABuscar->pcb->cantidadDePaginas++;
			list_replace(colaExec->elements, a, procesoABuscar);
		}
		a++;
	}
	printf("NO HAY PROCESO\n");
	exit(0);
}

int pideVariable(char *variable) {
	int i;
	for (i = 0;
			i < strlen((char*) t_archivoConfig->SHARED_VARS) / sizeof(char*);
			i++) {
		//TODO: mutex confignucleo
		if (strcmp((char*) t_archivoConfig->SHARED_VARS[i], variable) == 0) {
			return atoi(t_archivoConfig->SHARED_VARS_INIT[i]);
		}
	}
	printf("No encontre variable %s %d id, exit\n", variable, strlen(variable));
//TODO abortar
}

void escribeVariable(char *variable, int valor) {
	int i;
	char str[15];
	sprintf(str, "%d", valor);
	for (i = 0;
			i < strlen((char*) t_archivoConfig->SHARED_VARS) / sizeof(char*);
			i++) {
		if (strcmp((char*) t_archivoConfig->SHARED_VARS[i], variable) == 0) {
			memcpy((t_archivoConfig->SHARED_VARS_INIT[i]), str, sizeof(int));
			return;
		}
	}
	printf("No encontre VAR %s id, exit\n", variable);
	free(variable);
//TODO abortar

}

int pideSemaforo(char *semaforo) {
	int i;
//printf("NUCLEO: pide sem %s\n", semaforo);

	for (i = 0; i < strlen((char*) t_archivoConfig->SEM_IDS) / sizeof(char*);
			i++) {
		if (strcmp((char*) t_archivoConfig->SEM_IDS[i], semaforo) == 0) {
			return atoi(t_archivoConfig->SEM_INIT[i]);
		}
	}
	//printf("No encontre SEM id, exit\n");
	//exit(0);
}

void escribeSemaforo(char *semaforo, int valor) {
	int i;
	char str[15];
	sprintf(str, "%d", valor);
//printf("NUCLEO: pide sem %s\n", semaforo);

	for (i = 0; i < strlen((char*) t_archivoConfig->SEM_IDS) / sizeof(char*);
			i++) {
		if (strcmp((char*) t_archivoConfig->SEM_IDS[i], semaforo) == 0) {

			//if (config_nucleo->VALOR_SEM[i] == -1) {return &config_nucleo->VALOR_SEM[i];}
			memcpy((t_archivoConfig->SEM_INIT[i]), str, sizeof(int));
			return;
		}
	}
	//printf("No encontre SEM id, exit\n");
	//exit(0);
}

void liberaSemaforo(char *semaforo) {
	int i;
	proceso *proceso;

	for (i = 0; i < strlen((char*) t_archivoConfig->SEM_IDS) / sizeof(char*);
			i++) {
		if (strcmp((char*) t_archivoConfig->SEM_IDS[i], semaforo) == 0) {

			if (list_size(colas_semaforos[i]->elements)) {
				sacarSemaforosDesbloqueados(semaforo);
				proceso = queue_pop(colas_semaforos[i]);
				pthread_mutex_lock(&mutexColaReady);
				queue_push(colaReady, proceso);
				pthread_mutex_unlock(&mutexColaReady);
				sem_post(&semReady);
			} else {
				//esto deberia sumar 1
				int valor = 1 + atoi(t_archivoConfig->SEM_INIT[i]);
				char str[15];
				sprintf(str, "%d", valor);
				memcpy((t_archivoConfig->SEM_INIT[i]), str, sizeof(int));
			}

			return;
		}
	}
	//printf("No encontre SEM id, exit\n");
	//exit(0);
}

void bloqueoSemaforo(proceso *proceso, char *semaforo) {
	int i;

	for (i = 0; i < strlen((char*) t_archivoConfig->SEM_IDS) / sizeof(char*);
			i++) {
		if (strcmp((char*) t_archivoConfig->SEM_IDS[i], semaforo) == 0) {

			queue_push(colas_semaforos[i], proceso);
			return;

		}
	}
	//printf("No encontre SEM id, exit\n");
	// exit(0);
}

void compactarPaginaHeap(int pagina, int pid) {
	int offset = 0;
	HeapMetaData actual;
	HeapMetaData siguiente;
	HeapMetaData* buffer = malloc(sizeof(HeapMetaData));

	actual.size = 0;

	while (offset < MARCOS_SIZE
			&& offset + sizeof(HeapMetaData) + actual.size
					> MARCOS_SIZE - sizeof(HeapMetaData)) {
		int tamanoLectura = sizeof(HeapMetaData);
		void* lecturaPagina = malloc(sizeof(datosAdminHeap) + 3 * sizeof(int));
		memcpy(lecturaPagina, &pagina, sizeof(processID));
		memcpy(lecturaPagina + 4, &offset, sizeof(int));
		memcpy(lecturaPagina + 8, &tamanoLectura, sizeof(int));
		memcpy(lecturaPagina + 12, &pid, sizeof(int));
		Serializar(METADATALEIDA, 16, lecturaPagina, clienteMEM);
		sem_wait(&semMetaDataLeida);

		memcpy(buffer, metaDataLeida, sizeof(HeapMetaData));
		free(metaDataLeida);
		free(lecturaPagina);
		actual.isFree = buffer->isFree;
		actual.size = buffer->size;

		void* lecturaPagina2 = malloc(sizeof(datosAdminHeap) + 3 * sizeof(int));
		int offsetAux = offset + sizeof(HeapMetaData) + actual.size;
		memcpy(lecturaPagina, &pagina, sizeof(processID));
		memcpy(lecturaPagina + 4, &offsetAux, sizeof(int));
		memcpy(lecturaPagina + 8, &tamanoLectura, sizeof(int));
		memcpy(lecturaPagina + 12, &pid, sizeof(int));
		Serializar(METADATALEIDA, 16, lecturaPagina, clienteMEM);
		sem_wait(&semMetaDataLeida);

		memcpy(buffer, metaDataLeida, sizeof(HeapMetaData));
		free(metaDataLeida);
		free(lecturaPagina2);

		siguiente.isFree = buffer->isFree;
		siguiente.size = buffer->size;
		if (actual.isFree == -1 && siguiente.isFree == -1) {

			actual.size = actual.size + sizeof(HeapMetaData) + siguiente.size;
			memcpy(buffer, &actual, sizeof(HeapMetaData));

			int tamanoAUx = sizeof(HeapMetaData);
			void* envioPagina2 = malloc(sizeof(HeapMetaData) + 4 * sizeof(int));
			memcpy(envioPagina2, &pid, sizeof(int));
			memcpy(envioPagina2 + 4, &pagina, sizeof(int));
			memcpy(envioPagina2 + 8, &tamanoAUx, sizeof(int));
			memcpy(envioPagina2 + 12, &offset, sizeof(int));
			memcpy(envioPagina2 + 16, buffer, tamanoAUx);
			Serializar(PAGINA, sizeof(HeapMetaData) + 4 * sizeof(int),
					envioPagina2, clienteMEM);
			sem_wait(&semPaginaEnviada);
			free(envioPagina2);

		} else {
			offset += sizeof(HeapMetaData) + actual.size;
		}
	}
	free(buffer);
}

int paginaHeapBloqueSuficiente(int posicionPaginaHeap, int pagina, int pid,
		int size) {
	printf("Pagina Heap Bloque Suficiente\n");
	int i = 0;

	HeapMetaData auxBloque;
	void *buffer = malloc(sizeof(HeapMetaData));

	while (i < MARCOS_SIZE) {

		int tamanoLectura = sizeof(HeapMetaData);
		void* lecturaPagina = malloc(sizeof(datosAdminHeap) + 3 * sizeof(int));
		memcpy(lecturaPagina, &pagina, sizeof(processID));
		memcpy(lecturaPagina + 4, &i, sizeof(int));
		memcpy(lecturaPagina + 8, &tamanoLectura, sizeof(int));
		memcpy(lecturaPagina + 12, &pid, sizeof(int));
		Serializar(METADATALEIDA, 16, lecturaPagina, clienteMEM);
		sem_wait(&semMetaDataLeida);

		memcpy(buffer, metaDataLeida, sizeof(HeapMetaData));
		free(metaDataLeida);
		free(lecturaPagina);
		memcpy(&auxBloque, buffer, sizeof(HeapMetaData));

		if (auxBloque.size >= size + sizeof(HeapMetaData)
				&& auxBloque.isFree == -1) {
			printf("Pagina Heap Bloque Suficiente\n");
			free(buffer);
			return i + 8;
		}

		else {
			i = i + sizeof(HeapMetaData) + auxBloque.size;
		}
	}
	free(buffer);
	return -1;
}

datosHeap *verificarEspacioLibreHeap(int size, int pid) {
	int i = 0;
	datosHeap* puntero = malloc(sizeof(datosHeap));
	datosAdminHeap* aux;
	puntero->pagina = -1;

	pthread_mutex_lock(&mutexListaAdminHeap);
	while (i < list_size(listaAdmHeap)) {
		aux = (datosAdminHeap*) list_get(listaAdmHeap, i);
		pthread_mutex_unlock(&mutexListaAdminHeap);

		if (aux->tamanoDisponible >= size + sizeof(datosAdminHeap)
				&& aux->pid == pid) {
			/**TODO: Mutex para compactar?*/
			compactarPaginaHeap(aux->numeroPagina, aux->pid);
			puntero->offset = paginaHeapBloqueSuficiente(i, aux->numeroPagina,
					aux->pid, size);
			if (puntero->offset > 0) {
				puntero->pagina = aux->numeroPagina;
				break;
			}
		}
		i++;
	}
	pthread_mutex_unlock(&mutexListaAdminHeap);
	return puntero;
}

void reservarBloqueHeap(int pid, int size, datosHeap* puntero) {

	HeapMetaData auxBloque;
	datosAdminHeap* aux = malloc(sizeof(datosAdminHeap));
	int i = 0;
	int sizeLibreViejo;

	pthread_mutex_lock(&mutexListaAdminHeap);
	while (i < list_size(listaAdmHeap)) {
		aux = list_get(listaAdmHeap, i);
		if (aux->numeroPagina == puntero->pagina && aux->pid == pid) {
			if (size + sizeof(HeapMetaData) > aux->tamanoDisponible) {
				pthread_mutex_unlock(&mutexListaAdminHeap);
				printf("\nEntre a un error\n");

			} else {
				aux->tamanoDisponible = aux->tamanoDisponible - size
						- sizeof(HeapMetaData);
				list_replace(listaAdmHeap, i, aux);
				break;
			}
		}
		i++;
	}
	pthread_mutex_unlock(&mutexListaAdminHeap);
	int tamanoLectura = sizeof(HeapMetaData);
	free(aux);
	int otroOffset = puntero->offset - 8; // TODO: FIJARSE SI NO ES +8

	void* lecturaPagina = malloc(sizeof(HeapMetaData) + 3 * sizeof(int));
	memcpy(lecturaPagina, &puntero->pagina, sizeof(processID));
	memcpy(lecturaPagina + 4, &otroOffset, sizeof(int));
	memcpy(lecturaPagina + 8, &tamanoLectura, sizeof(int));
	memcpy(lecturaPagina + 12, &pid, sizeof(int));
	Serializar(METADATALEIDA, 16, lecturaPagina, clienteMEM);
	sem_wait(&semMetaDataLeida);
	memcpy(&auxBloque, metaDataLeida, sizeof(HeapMetaData));
	free(lecturaPagina);
	sizeLibreViejo = auxBloque.size;
	auxBloque.isFree = 1;
	auxBloque.size = size;
	memcpy(metaDataLeida, &auxBloque, sizeof(HeapMetaData));
	int tamanoAUx = sizeof(HeapMetaData);
	void* envioPagina = malloc(sizeof(HeapMetaData) + 4 * sizeof(int));
	memcpy(envioPagina, &pid, sizeof(processID));
	memcpy(envioPagina + 4, &puntero->pagina, sizeof(int));
	memcpy(envioPagina + 8, &tamanoAUx, sizeof(int));
	memcpy(envioPagina + 12, &otroOffset, sizeof(int));
	memcpy(envioPagina + 16, metaDataLeida, sizeof(HeapMetaData));
	Serializar(PAGINA, sizeof(HeapMetaData) + 4 * sizeof(int), envioPagina,
			clienteMEM);
	sem_wait(&semPaginaEnviada);

	auxBloque.isFree = -1;
	auxBloque.size = sizeLibreViejo - size - sizeof(HeapMetaData);
	tamanoAUx = sizeof(HeapMetaData);
	int nuevoOffset = otroOffset + sizeof(HeapMetaData) + size;
	memcpy(metaDataLeida, &auxBloque, sizeof(HeapMetaData));
	void* envioPagina2 = malloc(sizeof(HeapMetaData) + 4 * sizeof(int));
	memcpy(envioPagina2, &pid, sizeof(processID));
	memcpy(envioPagina2 + 4, &puntero->pagina, sizeof(int));
	memcpy(envioPagina2 + 8, &tamanoAUx, sizeof(int));
	memcpy(envioPagina2 + 12, &nuevoOffset, sizeof(int));
	memcpy(envioPagina2 + 16, metaDataLeida, tamanoAUx);
	Serializar(PAGINA, sizeof(HeapMetaData) + 4 * sizeof(int), envioPagina2,
			clienteMEM);
	sem_wait(&semPaginaEnviada);
	free(buffer);
	free(envioPagina);
	free(envioPagina2);
	free(metaDataLeida);

}

void reservarPaginaHeap(int pid, int pagina) { //Reservo una página de heap nueva para el proceso
	HeapMetaData aux;
	int tamanoAux = sizeof(HeapMetaData);
	int offsetAux = 0;
	void* buffer = malloc(sizeof(HeapMetaData));
	aux.isFree = -1;
	aux.size = MARCOS_SIZE - sizeof(HeapMetaData);
	memcpy(buffer, &aux, sizeof(HeapMetaData));
	Serializar(PROCESOPIDEHEAP, 4, &pid, clienteMEM);
	sem_wait(&semMemoriaReservoHeap);
	void* envioPagina = malloc(sizeof(HeapMetaData) + 4 * sizeof(int));
	memcpy(envioPagina, &pid, sizeof(processID));
	memcpy(envioPagina + 4, &pagina, sizeof(int));
	memcpy(envioPagina + 8, &tamanoAux, sizeof(int));
	memcpy(envioPagina + 12, &offsetAux, sizeof(int));
	memcpy(envioPagina + 16, buffer, sizeof(HeapMetaData));
	Serializar(PAGINA, sizeof(HeapMetaData) + 4 * sizeof(int), envioPagina,
			clienteMEM);
	sem_wait(&semPaginaEnviada);

	datosAdminHeap* bloqueAdmin = malloc(sizeof(datosAdminHeap));
	bloqueAdmin->numeroPagina = pagina;
	bloqueAdmin->pid = pid;
	bloqueAdmin->tamanoDisponible = aux.size;

	list_add(listaAdmHeap, bloqueAdmin);
	free(buffer);
	free(envioPagina);
}

datosHeap* procesoPideHeap(int pid, int tamano) {
	datosHeap * puntero;
	//TODO contar para memory leaks
	if (tamano > MARCOS_SIZE - sizeof(HeapMetaData) * 2) {
		//TODO: manejar errores de pedir heap
		return 0;
	}
	puntero = verificarEspacioLibreHeap(tamano, pid);
	if (puntero->pagina == -1) {
		//TODO: REVISAR ESTO
		puntero->pagina = ultimaPaginaPid[pid] + 1;
		ultimaPaginaPid[processID] += 1;

		pthread_mutex_lock(&mutexMemoria);
		reservarPaginaHeap(pid, puntero->pagina);
		pthread_mutex_unlock(&mutexMemoria);
		puntero->offset = 8;

		//TODO manejar si ya no hay mas recursos
	}
	pthread_mutex_lock(&mutexMemoria);
	reservarBloqueHeap(pid, tamano, puntero);
	pthread_mutex_unlock(&mutexMemoria);
	sem_post(&semTerminoDataHeap);
	return puntero;
}

void procesoLiberaHeap(int pid, int pagina, int offsetPagina) {
	datosAdminHeap* aux = malloc(sizeof(datosAdminHeap));
	HeapMetaData bloque;

	void *buffer = malloc(sizeof(HeapMetaData));

	pthread_mutex_lock(&mutexMemoria);
	int tamanoLectura = sizeof(HeapMetaData);
	void* lecturaPagina = malloc(sizeof(HeapMetaData) + 3 * sizeof(int));
	memcpy(lecturaPagina, &pagina, sizeof(processID));
	memcpy(lecturaPagina + 4, &offsetPagina, sizeof(int));
	memcpy(lecturaPagina + 8, &tamanoLectura, sizeof(int));
	memcpy(lecturaPagina + 12, &pid, sizeof(int));
	Serializar(METADATALEIDA, 16, lecturaPagina, clienteMEM);
	sem_wait(&semMetaDataLeida);
	memcpy(buffer, metaDataLeida, sizeof(HeapMetaData));
	free(metaDataLeida);
	free(lecturaPagina);
	pthread_mutex_unlock(&mutexMemoria);

	memcpy(&bloque, buffer, sizeof(HeapMetaData));

	bloque.isFree = -1;
	/*TODO: Poder saber bien cuanto estoy liberando*/
	printf("\n\nEstoy liberando:%d\n\n", bloque.size);

	memcpy(buffer, &bloque, sizeof(HeapMetaData));

	pthread_mutex_lock(&mutexMemoria);
	int tamanoAux = sizeof(HeapMetaData);
	void* envioPagina = malloc(sizeof(HeapMetaData) + 4 * sizeof(int));
	memcpy(envioPagina, &pid, sizeof(processID));
	memcpy(envioPagina + 4, &pagina, sizeof(int));
	memcpy(envioPagina + 8, &tamanoAux, sizeof(int));
	memcpy(envioPagina + 12, &offsetPagina, sizeof(int));
	memcpy(envioPagina + 16, buffer, sizeof(HeapMetaData));
	Serializar(PAGINA, sizeof(HeapMetaData) + 4 * sizeof(int), envioPagina,
			clienteMEM);
	sem_wait(&semPaginaEnviada);
	pthread_mutex_unlock(&mutexMemoria);
	free(envioPagina);
	pthread_mutex_lock(&mutexListaAdminHeap);
	while (i < list_size(listaAdmHeap)) {
		aux = list_get(listaAdmHeap, i);
		if (aux->numeroPagina == pagina && aux->pid == pid) {
			aux->tamanoDisponible = aux->tamanoDisponible + bloque.size;
			list_replace(listaAdmHeap, i, aux);
			break;
		}
		i++;
	}
	pthread_mutex_unlock(&mutexListaAdminHeap);
}

void abortar(proceso * proceso, int exitCode) {
	pthread_mutex_lock(&mutexColaCpu);
	queue_push(colaCpu, proceso->socketCPU);
	pthread_mutex_unlock(&mutexColaCpu);
	proceso->pcb->exitCode = exitCode;
	sem_post(&semCpu);
	//TODO revisar semcpu y semgradomulti
	void* envio = malloc(8);
	memcpy(envio, &proceso->pcb->programId, 4);
	memcpy(envio + 4, &exitCode, 4);
	Serializar(ABORTOSTACKOVERFLOW, 8, envio, proceso->socketCONSOLA);
	pthread_mutex_lock(&mutexColaExit);
	destruirCONTEXTO(proceso->pcb);
	queue_push(colaExit, proceso);
	pthread_mutex_unlock(&mutexColaExit);
	free(envio);
//TODO : liberar recursos de memoria
}

void abortarProgramaPorConsola(int pid, int codigo) {
	proceso* unProceso;
	bool esMiPid(void * entrada) {
		proceso * unproceso = (proceso *) entrada;
		return unproceso->pcb->programId == pid;
	}
	pthread_mutex_lock(&mutexColaReady);
	unProceso = (proceso*) list_remove_by_condition(colaReady->elements,
			esMiPid);
	pthread_mutex_unlock(&mutexColaReady);
	if (unProceso != NULL) {
		log_info(logger, "NUCLEO: Abortado x consola, en ready. Pre wait");
		printf("aborte pid: %d con codigo %d\n", unProceso->pcb->programId,
				codigo);
		//sem_post(&semCpu);
		log_info(logger, "NUCLEO: post wait");
		abortar(unProceso, codigo);
	} else {
		pthread_mutex_lock(&mutexColaEx);
		unProceso = (proceso*) list_find(colaExec->elements, esMiPid);
		pthread_mutex_unlock(&mutexColaEx);

		if (unProceso != NULL) {
			pthread_mutex_lock(&mutexColaEx);
			unProceso = (proceso*) list_remove_by_condition(colaExec->elements,
					esMiPid);
			pthread_mutex_unlock(&mutexColaEx);
			printf("aborte pid: %d con codigo %d\n", unProceso->pcb->programId,
					codigo);
			abortar(unProceso, codigo);
			//TODO esperar a que termine
			log_info(logger,
					"NUCLEO: Abortado x consola, en exec, espero que termine de trabajar");
			unProceso->abortado = true;
		} else {
			int i;
			for (i = 0;
					i < strlen((char*) t_archivoConfig->SEM_IDS) / sizeof(char*);
					i++) {

				unProceso = (proceso*) list_remove_by_condition(
						colas_semaforos[i]->elements, esMiPid);
				if (unProceso != NULL)
					break;

			}
			if (unProceso != NULL) {
				printf("aborte pid: %d con codigo %d\n",
						unProceso->pcb->programId, codigo);
				log_info(logger, "NUCLEO: Abortado x consola, en semaforo");
				abortar(unProceso, codigo);

			} else {
				//no hay en exec, lo busco en semaforos
				int i;
				for (i = 0;
						i
								< strlen((char*) t_archivoConfig->SEM_IDS)
										/ sizeof(char*); i++) {

					unProceso = (proceso*) list_remove_by_condition(
							colas_semaforos[i]->elements, esMiPid);
					if (unProceso != NULL)
						break;

				}
				if (unProceso != NULL) {
					printf("aborte pid: %d con codigo %d\n",
							unProceso->pcb->programId, codigo);
					log_info(logger, "NUCLEO: Abortado x consola, en semaforo");
					abortar(unProceso, codigo);
				}
			}
		}
	}
}

