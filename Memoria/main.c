#include "main.h"
#define ARCHIVOLOG "/home/utnso/Log/logMemoria.txt"
archivoConfigMemoria* t_archivoConfig;
t_config *config;
struct sockaddr_in direccionServidor;
int32_t servidor;
int32_t activado;
int32_t cliente;
int noIMporta = 0;
int32_t header;
int clienteCpu;
struct sockaddr_in direccionCliente;
uint32_t tamanoDireccion;
char* buffer;
int32_t tamanoPaquete;
int32_t opcion;
cache cache1;
infoNodoCache* punteroCache;
cacheLru* punteroUsos;
int32_t frameCache = 0;
frame frameGeneral;
int32_t tamanoFrame;
cacheLru nodoUso;
int32_t ultimaPaginaPid[100];
//infoTablaMemoria tablaMemoria[500];
infoTablaMemoria* punteroMemoria;
int32_t indiceTabla = 0;
int32_t indiceCache = 0;
infoTablaMemoria nodoTablaMemoria;
int32_t numeroPagina = 0;
int32_t pidAnt = -1;
int32_t pidAntCache = -1;
infoNodoCache nodoCache;
int32_t entradasPid = -1;
int32_t entradasCache = 0;
int32_t desplazamientoFrame = 0;

pthread_t hiloLevantarConexion;
int32_t idHiloLevantarConexion;

pthread_t hiloCpu;
pthread_t hiloKernel;
pthread_t hiloAtender;
int32_t idHiloCpu;

pthread_t hiloLeerComando;
int32_t idHiloLeerComando;
sem_t semPaginas;
pthread_mutex_t mutexProcesar;

void esperar() {

	usleep(t_archivoConfig->RETARDO_MEMORIA * 1000);
	log_info(log, "RETARDO");
}

int32_t main(int argc, char**argv) {
	configuracion(argv[1]);
	log = log_create(ARCHIVOLOG, "Memoria", 0, LOG_LEVEL_INFO);
	log_info(log, "Iniciando Memoria\n");
	infoTablaMemoria tablaMemoria[t_archivoConfig->MARCOS];
	infoNodoCache tablaCache[t_archivoConfig->ENTRADAS_CACHE];
	cacheLru tablaUsos[t_archivoConfig->ENTRADAS_CACHE];
	punteroMemoria = tablaMemoria;
	punteroCache = tablaCache;
	punteroUsos = tablaUsos;
	inicializarMemoria();
	inicializarOverflow(t_archivoConfig->MARCOS);
	//hacer un malloc , como memoria, hago un malloc del lugar de memoria ademas de tener una tabla
	crearFrameGeneral();
	crearCache();
	flush();
	idHiloLevantarConexion = pthread_create(&hiloLevantarConexion, NULL,
			levantarConexion, NULL);
	idHiloLeerComando = pthread_create(&hiloLeerComando, NULL, leerComando,
	NULL);

	pthread_join(hiloLevantarConexion, NULL);

	pthread_join(hiloLeerComando, NULL);
	return EXIT_SUCCESS;
}

void configuracion(char *dir) {
	t_archivoConfig = malloc(sizeof(archivoConfigMemoria));
	configuracionMemoria(t_archivoConfig, config, dir);

}

int32_t levantarConexion() {
	llenarSocketAdrr(&direccionServidor, t_archivoConfig->PUERTO);

	servidor = socket(AF_INET, SOCK_STREAM, 0);

	activado = 1;
	setsockopt(servidor, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

	if (bind(servidor, (void*) &direccionServidor, sizeof(direccionServidor))
			!= 0) {
		perror("Falló el bind");
		return 1;
	}

	log_info(log, "Estoy escuchando\n");
	listen(servidor, 100);

	conectarseConKernel();
	atenderConexionesCPu();
}

void conectarseConKernel() {
	cliente = accept(servidor, (void*) &direccionCliente, &tamanoDireccion);
	log_info(log, "Recibí una conexión en %d!!\n", cliente);
	int envio = t_archivoConfig->MARCOS_SIZE;
	Serializar(MEMORIA, 4, &envio, cliente);

	pthread_create(&hiloKernel, NULL, (void *) atenderKernel, NULL);
	//pthread_join(hiloKernel, NULL);
	return;
}

void atenderConexionesCPu() {
	while (1) {
		clienteCpu = accept(servidor, (void*) &direccionCliente,
				&tamanoDireccion);

		pthread_create(&hiloAtender, NULL, (void *) atenderCpu, clienteCpu);

		//pthread_join(hiloAtender, NULL);
	}

}

int atenderCpu(int socket) {
	log_info(log, "Recibí una conexión en %d!!\n", socket);
	int envio = t_archivoConfig->MARCOS_SIZE;
	Serializar(MEMORIA, 4, &envio, socket);
	while (1) {
		paquete* paqueteRecibido = Deserializar(socket);
		if (paqueteRecibido->header == -1 || paqueteRecibido->header == -2) {
			perror("El chabón se desconectó\n");
			return 1;
		}
		pthread_mutex_lock(&mutexProcesar);
		procesar(paqueteRecibido->package, paqueteRecibido->header,
				paqueteRecibido->size, socket);

		pthread_mutex_unlock(&mutexProcesar);

	}
	return 0;
}

int atenderKernel() {
	while (1) {
		paquete* paqueteRecibido = Deserializar(cliente);
		if (paqueteRecibido->header == -1 || paqueteRecibido->header == -2) {
			perror("El chabón se desconectó\n");
			return 1;
		}

		pthread_mutex_lock(&mutexProcesar);
		procesar(paqueteRecibido->package, paqueteRecibido->header,
				paqueteRecibido->size, cliente);

		pthread_mutex_unlock(&mutexProcesar);

	}
	return 0;
}

void leerComando() {
	while (1) {
		log_info(log, "\nIngrese comando\n"
				"1: dump\n"
				"2: flush\n"
				"3: size\n");
		scanf("%d", &opcion);
		switch (opcion) {
		case 1: {
			dump();
			break;
		}
		case 2: {
			flush();
			break;
		}
		case 3: {
			size();
			break;
		}

		case 4: {

			break;
		}
		case 5: {
			size();
			break;
		}
		case 6: {
			break;
		}

		}
	}
}

void procesar(char * paquete, int32_t id, int32_t tamanoPaquete, int32_t socket) {
	int numero_pagina, offset, tamanio, pid_actual;
	switch (id) {
	case ARCHIVO: {
		log_info(log, "%s", paquete);
		break;
	}
	case FILESYSTEM: {
		log_info(log, "Se conecto FS\n");
		break;
	}
	case KERNEL: {
		log_info(log, "Se conecto Kernel\n");
		break;
	}
	case CPU: {
		log_info(log, "Se conecto CPU\n");
		break;
	}
	case CONSOLA: {
		log_info(log, "Se conecto Consola\n");
		break;
	}
	case MEMORIA: {
		log_info(log, "Se conecto memoria\n");
		break;
	}
	case CODIGO: {
		break;
	}
	case TAMANO: {
		int32_t paginas = (int) (*paquete);

		if (paginas > 0) {
			//if (frameGeneral.tamanioDisponible - (paginas*20) >= 0){
			int32_t paginasNegativas = -paginas;
			sem_init(&semPaginas, 0, &paginasNegativas);
			int i;
			if (paginas * t_archivoConfig->MARCOS_SIZE
					> frameGeneral.tamanioDisponible) {
				Serializar(NOENTROPROCESO, 4, &noIMporta, socket);
				log_info(log,
						"el proceso no entra porque la memoria esta llena\n");
			} else {
				Serializar(ENTRAPROCESO, 4, &noIMporta, socket);
				log_info(log, "el proceso entra\n");
			}
			//}
		}
		break;
	}
	case INICIALIZARPROCESO: {
		int pid;
		int cantidadDePaginas;
		memcpy(&pid, paquete, 4);
		memcpy(&cantidadDePaginas, paquete + 4, 4);
		ultimaPaginaPid[pid] = cantidadDePaginas;
		inicializarPrograma(pid, cantidadDePaginas);
		break;
	}
	case PAGINA: {
		int32_t pid;
		int32_t numeroPagina;
		int32_t offset;
		int32_t tamano;

		//pagina[t_archivoConfiheaderg->MARCOS_SIZE] = '\0';
		memcpy(&pid, paquete, sizeof(int));
		memcpy(&numeroPagina, paquete + 4, sizeof(int));
		memcpy(&tamano, paquete + 8, sizeof(int));
		char *codigoPagina = malloc(tamano);
		memcpy(&offset, paquete + 12, sizeof(int));
		memcpy(codigoPagina, paquete + 16, tamano);
		escribirEnPagina(pid, numeroPagina, offset, tamano, codigoPagina);
		//log_info(log,"pagina: %s\n", pagina);
		//log_info(log,"pid: %d\n", pid);
		Serializar(PAGINAENVIADA, 4, &noIMporta, socket);
		//tamanoPaquete es la cantidad de paginas que necesito

		//almacernarPaginaEnFrame(pid, tamanoPaquete, paquete);
		break;
	}
	case LEERSENTENCIA: {
		int pid;
		char* contenido;
		memcpy(&numero_pagina, paquete, sizeof(int));
		memcpy(&offset, paquete + sizeof(int), sizeof(int));
		memcpy(&tamanio, paquete + sizeof(int) * 2, sizeof(int));
		memcpy(&pid, paquete + sizeof(int) * 3, sizeof(int));
		printf(
				"[LEERSENTENCIA]Quiero leer en la direccion: %d %d %d y le voy a enviar a socket: %d\n",
				numero_pagina, offset, tamanio, socket);
		int resultado = buscarNodoCache(pid, numero_pagina);
		if (resultado == -1) {
			contenido = leerDePagina(pid, numero_pagina, offset, tamanio);
			almacenarFrameEnCache(pid, numero_pagina);
			Serializar(id, tamanio, contenido, socket);
			free(contenido);
		} else {

			contenido = leerDeCache(pid, numero_pagina, offset, tamanio);
			log_info(log,
					"lei en cache pid %d pagina %d tamano %d contenido %s\n",
					pid, numero_pagina, tamanio, contenido);
			Serializar(id, tamanio, contenido, socket);
			free(contenido);
		}

		//log_info(log,"lei: %s\n", contenido);

		//ojo pid actual
		break;
	}
	case METADATALEIDA:
	case PROCESOLIBERAHEAP: {
		int pid;
		char* contenido;
		memcpy(&numero_pagina, paquete, sizeof(int));
		memcpy(&offset, paquete + sizeof(int), sizeof(int));
		memcpy(&tamanio, paquete + sizeof(int) * 2, sizeof(int));
		memcpy(&pid, paquete + sizeof(int) * 3, sizeof(int));
		printf(
				"[LEERSENTENCIA]Quiero leer en la direccion: %d %d %d y le voy a enviar a socket: %d\n",
				numero_pagina, offset, tamanio, socket);
		contenido = leerDePagina(pid, numero_pagina, offset, tamanio);

		log_info(log,
				"lei en MEMORIA pid %d pagina %d tamano %d contenido %s porque fue heap o metadata\n",
				pid, numero_pagina, tamanio, contenido);

		//log_info(log,"lei: %s\n", contenido);
		Serializar(id, tamanio, contenido, socket);
		free(contenido);
		//ojo pid actual
		break;
	}
	case LIBERARPAGINAS: {
		int pid;
		int pagina;
		memcpy(&pid, paquete, 4);
		memcpy(&pagina, paquete + 4, 4);
		liberarPaginaDeProceso(pid, pagina);
		break;
	}
	case DEREFERENCIAR: {
		int pid;
		char* contenido;
		memcpy(&numero_pagina, paquete, sizeof(int));
		memcpy(&offset, paquete + sizeof(int), sizeof(int));
		memcpy(&tamanio, paquete + sizeof(int) * 2, sizeof(int));

		memcpy(&pid, paquete + sizeof(int) * 3, sizeof(int));
		printf(
				"Quiero leer en la direccion: %d %d %d y le voy a enviar a socket: %d\n",
				numero_pagina, offset, offset, socket);
		int resultado = buscarNodoCache(pid, numero_pagina);
		if (resultado == -1) {
			contenido = leerDePagina(pid, numero_pagina, offset, tamanio);
			almacenarFrameEnCache(pid, numero_pagina);
		} else {
			contenido = leerDeCache(pid, numero_pagina, offset, tamanio);
			log_info(log,
					"lei en cache pid %d pagina %d tamano %d contenido %s\n",
					pid, numero_pagina, tamanio, contenido);
		}
		//log_info(log,"lei: %s\n", contenido);
		Serializar(DEREFERENCIAR, tamanio, contenido, socket);
		free(contenido);
		//ojo pid actual
		break;
	}
	case ESCRIBIRVARIABLE: {
		memcpy(&numero_pagina, paquete, sizeof(int));
		memcpy(&offset, paquete + sizeof(int), sizeof(int));
		memcpy(&tamanio, paquete + sizeof(int) * 2, sizeof(int));

		buffer = malloc(tamanio);

		memcpy(buffer, paquete + sizeof(int) * 3, tamanio);
		int pid;

		memcpy(&pid, paquete + sizeof(int) * 4, sizeof(int));

		escribirEnPagina(pid, numero_pagina, offset, tamanio, buffer);

		Serializar(ESCRIBIRVARIABLE, sizeof(int), &noIMporta, socket);
		sem_post(&semPaginas);
		free(buffer);
		break;
	}
	case PROCESOPIDEHEAP: {
		int pid;
		memcpy(&pid, paquete, sizeof(int));
		asignarPaginasAProceso(pid, 1);
		Serializar(MEMORIARESERVOHEAP, sizeof(int), &noIMporta, socket);
	}
	}

}

void crearFrameGeneral() {

	int32_t tamanioMarcos, cantidadMarcos;
	cantidadMarcos = t_archivoConfig->MARCOS;
	tamanioMarcos = t_archivoConfig->MARCOS_SIZE;
	tamanoFrame = tamanioMarcos;
	frameGeneral.id = 1;
	frameGeneral.tamanio = cantidadMarcos * tamanioMarcos;
	frameGeneral.tamanioDisponible = frameGeneral.tamanio;
	frameGeneral.tamanioOcupado = 0;
	frameGeneral.puntero = malloc(cantidadMarcos * tamanioMarcos);
	frameGeneral.punteroDisponible = frameGeneral.puntero;
	frameGeneral.framesLibres = cantidadMarcos;

}
void crearCache() {

	int32_t cantidadEntradas, tamanioMarcos;
	tamanioMarcos = t_archivoConfig->MARCOS_SIZE;
	cantidadEntradas = t_archivoConfig->ENTRADAS_CACHE;
	cache1.tamanio = cantidadEntradas * tamanioMarcos;
	cache1.tamanioDisponible = cache1.tamanio;
	cache1.puntero = malloc(cache1.tamanio);
	cache1.punteroDisponible = cache1.puntero;
	cache1.framesLibres = cantidadEntradas;
}
void inicializarMemoria() {
	int32_t i;
	for (i = 0; i <= t_archivoConfig->MARCOS; i++) {
		(punteroMemoria + i)->pid = 0;
		(punteroMemoria + i)->numeroPagina = 0;
	}
}
void dump() {
	t_log * log;
	log = log_create("dump.log", "Memoria", 0, LOG_LEVEL_INFO);
	log_info(log, "Tamanio de cache %d", t_archivoConfig->ENTRADAS_CACHE);
	log_info(log, "Tamanio disponible de cache %d",
			t_archivoConfig->ENTRADAS_CACHE * t_archivoConfig->MARCOS_SIZE);// 5hardcodeado
	int32_t i;
	for (i = 0; i <= 500; i++) {
		if ((punteroMemoria + i)->pid > 0) {
			log_info(log, "numero de frame %d", i);
			log_info(log, "pid %d", (punteroMemoria + i)->pid);
			log_info(log, "numero de pagina %d",
					(punteroMemoria + i)->numeroPagina);
		}
	}

}

void flush() {

	int i;
	for (i = 0; i < t_archivoConfig->ENTRADAS_CACHE; i++) {
		(punteroCache + i)->pid = 0;
		(punteroCache + i)->numeroPagina = 0;
	}

	return;
}

void size() {
	int32_t size;
	log_info(log, "size: 0 memoria 1 proceso\n ");
	scanf("%d", &size);
	switch (size) {
	case 0: {
		log_info(log, "tamanio total memoria %d\n", frameGeneral.tamanio);
		log_info(log, "tamanio disponible memoria %d\n",
				frameGeneral.tamanioDisponible);
		log_info(log, "tamanio ocupado memoria %d\n",
				frameGeneral.tamanioOcupado);
		break;
	}
	case 1: {
		break;
	}

	}
}

void inicializarPrograma(int32_t pid, int32_t cantPaginas) {
	int32_t i;
	esperar();
	for (i = 0; i <= cantPaginas - 1; i++) {

		int32_t frame = calcularPosicion(pid, i);
		int32_t libre;
		libre = estaLibre(frame);
		if (libre == 1) {

			nodoTablaMemoria.numeroPagina = i;
			nodoTablaMemoria.pid = pid;
			log_info(log, "el frame guardado fue %d\n", frame);
			punteroMemoria[frame] = nodoTablaMemoria;
			frameGeneral.framesLibres--;

		} else {
			int32_t frameLibre;
			frameLibre = buscarFrameLibre();
			agregarSiguienteEnOverflow(frame, frameLibre);
			nodoTablaMemoria.numeroPagina = i;
			nodoTablaMemoria.pid = pid;
			log_info(log, "el frame guardado por overflow fue %d\n", frameLibre);
			punteroMemoria[frameLibre] = nodoTablaMemoria;
			frameGeneral.framesLibres--;
		}
	}
}

void asignarPaginasAProceso(int32_t pid, int32_t cantPaginas) {
	esperar();
	if (frameGeneral.framesLibres > 0) {
		noIMporta = 1;
		int32_t i = ultimaPaginaPid[pid];
		ultimaPaginaPid[pid] += 1;
		int32_t frame = calcularPosicion(pid, i);
		int32_t libre;
		libre = estaLibre(frame);
		if (libre == 1) {

			nodoTablaMemoria.numeroPagina = i;
			nodoTablaMemoria.pid = pid;
			punteroMemoria[frame] = nodoTablaMemoria;
			frameGeneral.framesLibres--;

		} else {
			int32_t frameLibre;
			frameLibre = buscarFrameLibre();
			agregarSiguienteEnOverflow(frame, frameLibre);
			nodoTablaMemoria.numeroPagina = i;
			nodoTablaMemoria.pid = pid;
			punteroMemoria[frameLibre] = nodoTablaMemoria;
			frameGeneral.framesLibres--;

		}
	}

	else
		noIMporta = 0;
}

int32_t estaLibre(int32_t unFrame) {
	if ((punteroMemoria + unFrame)->pid == 0
			&& (punteroMemoria + unFrame)->numeroPagina == 0) {
		return 1;
	} else {
		return 0;
	}
}
int32_t buscarUltimaPag(int32_t pid) {
	int32_t ultimaPagina = 0;
	int32_t i;
	for (i = 0; i <= t_archivoConfig->MARCOS; i++) {
		if ((punteroMemoria + i)->pid == pid
				&& (punteroMemoria + i)->numeroPagina > ultimaPagina) {
			ultimaPagina = (punteroMemoria + i)->numeroPagina;
		}
	}
	return ultimaPagina;
}

int32_t buscarFrameLibre() {

	int32_t frameLibre = calcularPosicion(0, 0);
	return frameLibre;
}

void almacernarPaginaEnFrame(int32_t pid, int32_t tamanioBuffer, char* buffer) {
//SIEMPRE LE TIENE QUE LLEGAR TAMANIO<MARCOS_SIZE OJO
	esperar();
	memcpy(frameGeneral.punteroDisponible, buffer, tamanioBuffer);

	if (pid != pidAnt) {
		numeroPagina = 1;
		pidAnt = pid;
	}
//nodoTablaMemoria.puntero = frameGeneral.tamanioOcupado;
	nodoTablaMemoria.numeroPagina = numeroPagina;

//frameGeneral.punteroDisponible += t_archivoConfig->MARCOS_SIZE;
	frameGeneral.tamanioOcupado += t_archivoConfig->MARCOS_SIZE;
	frameGeneral.tamanioDisponible -= t_archivoConfig->MARCOS_SIZE;
	nodoTablaMemoria.pid = pid;
	punteroMemoria[indiceTabla] = nodoTablaMemoria;
	indiceTabla++;

	almacenarFrameEnCache(pid, numeroPagina);

	numeroPagina++;
//PROBAR

}
void liberarPaginaDeProceso(int32_t pid, int32_t pagina) {
	esperar();
	int32_t frameBorrar = buscarFrame(pid, pagina);
//int32_t i;
	/*
	 for(i = frameBorrar+1; i<=500; i++){
	 punteroMemoria[i-1] = punteroMemoria[i];
	 }
	 */
	log_info(log, "el frame liberado fue %d del pid %d\n", frameBorrar, pid);
	frameGeneral.framesLibres++;
	frameGeneral.tamanioOcupado -= t_archivoConfig->MARCOS_SIZE;
	frameGeneral.tamanioDisponible += t_archivoConfig->MARCOS_SIZE;
	(punteroMemoria + frameBorrar)->pid = 0;
	(punteroMemoria + frameBorrar)->numeroPagina = 0;
}

int32_t buscarFrame(int32_t pid, int32_t numeroPagina) {
	int32_t i;
	for (i = 0; i <= t_archivoConfig->MARCOS; i++) {
		if ((punteroMemoria + i)->pid == pid
				&& (punteroMemoria + i)->numeroPagina == numeroPagina) {
			return i;
		}
	}

	return -1;

}

char* leerDePagina(int32_t pid, int32_t pagina, int32_t offset, int32_t tamano) {
	esperar();
//int32_t unFrame = buscarFrame(pid, pagina);
	int32_t unFrame = calcularPosicion(pid, pagina);

	int32_t correcta = esPaginaCorrecta(unFrame, pid, pagina);
	char* contenido = malloc(tamano);
	if (correcta == 1) {
		int32_t desplazamiento = unFrame * t_archivoConfig->MARCOS_SIZE
				+ offset;
		memcpy(contenido, frameGeneral.puntero + desplazamiento, tamano);
		//contenido[tamano] = '\0';
		log_info(log,
				"lei en memoria pid %d pagina %d tamano %d contenido %s en memoria con espera\n",
				pid, pagina, tamano, contenido);
		return contenido;
	} else {

		int32_t colision = buscarEnOverflow(unFrame, pid, pagina);
		agregarSiguienteEnOverflow(unFrame, colision);
		int32_t desplazamiento = colision * t_archivoConfig->MARCOS_SIZE
				+ offset;
		memcpy(contenido, frameGeneral.puntero + desplazamiento, tamano);
		return contenido;
	}

}

void escribirEnPagina(int32_t pid, int32_t pagina, int32_t offset,
		int32_t tamano, char* contenido) {
	log_info(log,
			"escribi pid %d pagina %d offset %d tamano %d contenido %s en memoria con espera\n",
			pid, pagina, offset, tamano, contenido);
	esperar();
//int32_t unFrame = buscarFrame(pid, pagina);
	int32_t unFrame = calcularPosicion(pid, pagina);
	int32_t correcta = esPaginaCorrecta(unFrame, pid, pagina);
	if (correcta == 1) {
		int32_t desplazamiento = unFrame * t_archivoConfig->MARCOS_SIZE
				+ offset;
		memcpy(frameGeneral.puntero + desplazamiento, contenido, tamano);
	} else {

		int32_t colision = buscarEnOverflow(unFrame, pid, pagina);
		agregarSiguienteEnOverflow(unFrame, colision);
		int32_t desplazamiento = colision * t_archivoConfig->MARCOS_SIZE
				+ offset;
		memcpy(frameGeneral.puntero + desplazamiento, contenido, tamano);

	}

}

/*void inicializarProgramaCache(int32_t pid,int32_t cantPaginas){
 int32_t i;
 if(cantPaginas > t_archivoConfig->ENTRADAS_CACHE){
 for(i=0;i<= t_archivoConfig->ENTRADAS_CACHE-1;i++){
 nodoCache.pid = pid;
 nodoCache.numeroPagina=i+1;
 nodoCache.inicioContenido = t_archivoConfig->MARCOS_SIZE * indiceCache;
 nodoUso.pid = pid;
 nodoUso.pagina = i+1;
 nodoUso.uso = 0;
 punteroCache[indiceCache] = nodoCache;
 punteroUsos[indiceCache] = nodoUso;
 indiceCache++;
 }
 }
 else{
 for(i=0;i<=cantPaginas-1;i++){
 nodoCache.pid = pid;
 nodoCache.numeroPagina;
 nodoCache.inicioContenido = t_archivoConfig->MARCOS_SIZE * indiceCache;
 nodoUso.pid = pid;
 nodoUso.pagina = i+1;
 nodoUso.uso = 0;
 punteroCache[indiceCache] = nodoCache;
 punteroUsos[indiceCache] = nodoUso;
 indiceCache++;

 }
 }
 }


 */

void almacenarFrameEnCache(int32_t pid, int32_t pagina) {
	int tamanioBuffer = t_archivoConfig->MARCOS_SIZE;
	char * buffer = leerDePagina(pid, pagina, 0, tamanioBuffer);
	nodoCache.pid = pid;
	nodoCache.numeroPagina = pagina;
	nodoCache.inicioContenido = t_archivoConfig->MARCOS_SIZE * indiceCache;
	nodoUso.pid = pid;
	nodoUso.pagina = pagina;
	nodoUso.uso = 0;
	if (entradasCache < t_archivoConfig->ENTRADAS_CACHE
			&& t_archivoConfig->CACHE_X_PROC > 0) {
		if (buscarPidCache(pid) == 0) {

			memcpy(cache1.punteroDisponible, buffer, tamanioBuffer);
			entradasPid = 0;
			punteroCache[indiceCache] = nodoCache;
			punteroUsos[indiceCache] = nodoUso;
			indiceCache++;
			entradasPid++;
			entradasCache++;
			cache1.punteroDisponible += t_archivoConfig->MARCOS_SIZE;
			log_info(log,
					"almacene en cache pid %d pagina %d tamano %d contenido %s en memoria con espera\n",
					pid, pagina, tamanioBuffer, buffer);

		} else if (buscarPidCache(pid) == 1
				&& entradasPid < t_archivoConfig->CACHE_X_PROC) {
			memcpy(cache1.punteroDisponible, buffer, tamanioBuffer);
			cache1.punteroDisponible += t_archivoConfig->MARCOS_SIZE;
			punteroCache[indiceCache] = nodoCache;
			punteroUsos[indiceCache] = nodoUso;
			indiceCache++;
			entradasPid++;
			entradasCache++;
			log_info(log,
					"almacene en cache pid %d pagina %d tamano %d contenido %s en memoria con espera\n",
					pid, pagina, tamanioBuffer, buffer);

		} else {
			remplazoLru(nodoCache, buffer);

		}

	}
	free(buffer);
}

void escribirEnCache(int32_t pid, int32_t pagina, int32_t offset,
		int32_t tamano, char* contenido) {
	int32_t posicionCache;
	posicionCache = buscarPosicionContenido(pid, pagina);
	int32_t desplazamiento = posicionCache + offset;
	memcpy(cache1.punteroDisponible + desplazamiento, contenido, tamano);
	log_info(log,
			"almacene en cache pid %d pagina %d tamano %d contenido %s en memoria con espera\n",
			pid, pagina, tamano, contenido);

	/*nodoCache.pid = pid;
	 nodoCache.numeroPagina = pagina;
	 nodoCache.inicioContenido = buscarNodoCache(pid,pagina)*t_archivoConfig->MARCOS_SIZE + offset; // offset donde se va aescribir ese contenido en cache
	 */
// corregit el offset contenido = tamanoFrame * i ; i++;
//
//contenido = leerDePagina(pid,pagina,offset,tamano);
	/*
	 if(entradasCache < t_archivoConfig->ENTRADAS_CACHE){
	 if (buscarPidCache(pid)==0) {
	 //revisar si mover puntero para memcpy
	 memcpy(cache1.punteroDisponible+nodoCache.inicioContenido, contenido, tamano);
	 entradasPid =0;
	 punteroCache[indiceCache] = nodoCache;
	 indiceCache++;
	 entradasPid++;
	 entradasCache++;
	 }
	 else if(buscarPidCache(pid)==1 && entradasPid < t_archivoConfig->CACHE_X_PROC){
	 memcpy(cache1.punteroDisponible+nodoCache.inicioContenido, contenido, tamano);
	 punteroCache[indiceCache] = nodoCache;
	 indiceCache++;
	 entradasPid++;
	 entradasCache++;
	 }
	 else
	 {
	 log_info(log,"proceso no ingreso a cache por maximo de entradas");
	 }
	 }
	 else
	 {
	 remplazoLru(nodoCache, contenido);

	 }
	 */
}

char* leerDeCache(int32_t pid, int32_t pagina, int32_t offset, int32_t tamano) {
	char* contenido = malloc(tamano);

	int32_t offsetContenido;
	int32_t i;
	for (i = 0; i <= t_archivoConfig->ENTRADAS_CACHE; i++) {
		if ((punteroCache + i)->pid == pid
				&& (punteroCache + i)->numeroPagina == pagina) {
			offsetContenido = (punteroCache + i)->inicioContenido;
			(punteroUsos + i)->uso++;
			memcpy(contenido, cache1.puntero + offsetContenido + offset,
					tamano);
			log_info(log, "lei en cache pid %d pagina %d contenido %s \n", pid,
					pagina, contenido);
			return contenido;

		}

	}
}

int32_t buscarPidCache(int32_t pid) {
	int32_t i;
	for (i = 0; i <= t_archivoConfig->ENTRADAS_CACHE; i++) {
		if ((punteroCache + i)->pid == pid) {
			return 1;
		}

	}
	return 0;
}
int32_t buscarNodoCache(int32_t pid, int32_t pagina) {
	int32_t i;
	if (t_archivoConfig->CACHE_X_PROC == 0) {
		return -1;
	}
	for (i = 0; i <= t_archivoConfig->ENTRADAS_CACHE; i++) {
		if ((punteroCache + i)->pid == pid
				&& (punteroCache + i)->numeroPagina == pagina) {
			return i;
		}
	}

	return -1;

}

int32_t buscarPosicionContenido(int32_t pid, int32_t pagina) {
	int32_t i;
	for (i = 0; i <= t_archivoConfig->ENTRADAS_CACHE; i++) {
		if ((punteroCache + i)->pid == pid
				&& (punteroCache + i)->numeroPagina == pagina) {
			return (punteroCache + i)->inicioContenido;
		}
	}

	return -1;

}

void remplazoLru(infoNodoCache nodoCache, char* contenido) {
	ordenarPorUso();
	int32_t i;
	cacheLru menosUsado = punteroUsos[0];
	int32_t posicionCache;
	posicionCache = buscarNodoCache(menosUsado.pid, menosUsado.pagina);
	for (i = posicionCache + 1; i <= t_archivoConfig->ENTRADAS_CACHE; i++) {
		punteroCache[i - 1] = punteroCache[i];
	}
	memcpy(cache1.puntero + posicionCache, contenido,
			t_archivoConfig->MARCOS_SIZE);
	cache1.punteroDisponible += t_archivoConfig->MARCOS_SIZE;
	log_info(log, "almacene en cache pid %d pagina %d contenido %s\n",
			nodoCache.pid, nodoCache.numeroPagina, contenido);

}
void ordenarPorUso() {
	int32_t i;
	int32_t x;
	cacheLru aux;
	for (i = 0; i <= t_archivoConfig->ENTRADAS_CACHE; i++) {
		for (x = i + 1; x <= t_archivoConfig->ENTRADAS_CACHE - 1; x++) {
			if ((punteroUsos + i)->uso > (punteroUsos + x)->uso) {
				aux = punteroUsos[i];
				punteroUsos[i] = punteroUsos[x];
				punteroUsos[x] = aux;
			}
		}
	}
}

/* Función Hash */
unsigned int calcularPosicion(int pid, int num_pagina) {
	char str1[20];
	char str2[20];
	sprintf(str1, "%d", pid);
	sprintf(str2, "%d", num_pagina);
	strcat(str1, str2);
	CANTIDAD_DE_MARCOS = t_archivoConfig->MARCOS;
	unsigned int indice = atoi(str1) % CANTIDAD_DE_MARCOS;
	return indice;
}

/* Inicialización vector overflow. Cada posición tiene una lista enlazada que guarda números de frames.
 * Se llenará a medida que haya colisiones correspondientes a esa posición del vector. */
void inicializarOverflow(int cantidad_de_marcos) {
	overflow = malloc(sizeof(t_list*) * cantidad_de_marcos);
	int i;
	for (i = 0; i < cantidad_de_marcos; ++i) { /* Una lista por frame */
		overflow[i] = list_create();
	}
}

/* En caso de colisión, busca el siguiente frame en el vector de overflow.
 * Retorna el número de frame donde se encuentra la página. */
int buscarEnOverflow(int indice, int pid, int pagina) {
	int i = 0;
	for (i = 0; i < list_size(overflow[indice]); i++) {
		if (esPaginaCorrecta(list_get(overflow[indice], i), pid, pagina)) {
			return list_get(overflow[indice], i);
		}
	}
}

/* Agrega una entrada a la lista enlazada correspondiente a una posición del vector de overflow */
void agregarSiguienteEnOverflow(int pos_inicial, int nro_frame) {
	list_add(overflow[pos_inicial], nro_frame);
}

/* Elimina un frame de la lista enlazada correspondiente a una determinada posición del vector de overflow  */
void borrarDeOverflow(int posicion, int frame) {
	int i = 0;
	int index_frame;

	for (i = 0; i < list_size(overflow[posicion]); i++) {
		if (frame == (int) list_get(overflow[posicion], i)) {
			index_frame = i;
			i = list_size(overflow[posicion]);
		}
	}

	list_remove(overflow[posicion], index_frame);
}

/* A implementar por el alumno. Devuelve 1 a fin de cumplir con la condición requerida en la llamada a la función */
int esPaginaCorrecta(int pos_candidata, int pid, int pagina) {
	if ((punteroMemoria + pos_candidata)->pid == pid
			&& (punteroMemoria + pos_candidata)->numeroPagina == pagina) {
		return 1;
	} else {
		return 0;
	}

}

