#include "main.h"
archivoConfigMemoria* t_archivoConfig;
t_config *config;
struct sockaddr_in direccionServidor;
int32_t servidor;
int32_t activado;
int32_t cliente;
int noIMporta;
int32_t header;
int clienteCpu;
struct sockaddr_in direccionCliente;
uint32_t tamanoDireccion;
char* buffer;
int32_t tamanoPaquete;
int32_t opcion;
cache cache1;

frame frameGeneral;
int32_t tamanoFrame;

infoTablaMemoria tablaMemoria[500];
int32_t indiceTabla = 0;
infoTablaMemoria nodoTablaMemoria;
int32_t numeroPagina = 0;
int32_t pidAnt = -1;

pthread_t hiloLevantarConexion;
int32_t idHiloLevantarConexion;

pthread_t hiloCpu;
pthread_t hiloKernel;
pthread_t hiloAtender;
int32_t idHiloCpu;

pthread_t hiloLeerComando;
int32_t idHiloLeerComando;

int32_t main(int argc, char**argv) {

	printf("memoria \n");
	configuracion(argv[1]);
	crearFrameGeneral();
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

	printf("Estoy escuchando\n");
	listen(servidor, 100);

	conectarseConKernel();
	atenderConexionesCPu();
}

void conectarseConKernel() {
	cliente = accept(servidor, (void*) &direccionCliente, &tamanoDireccion);
	printf("Recibí una conexión en %d!!\n", cliente);
	int envio = t_archivoConfig->MARCOS_SIZE;
	Serializar(MEMORIA, 4, &envio, cliente);

	pthread_create(&hiloKernel, NULL, (void *) atenderKernel, NULL);
	//pthread_join(hiloKernel, NULL);
	return ;
}

void atenderConexionesCPu() {
	while (1) {
		clienteCpu = accept(servidor, (void*) &direccionCliente,
				&tamanoDireccion);
		printf("Recibí una conexión en %d!!\n", clienteCpu);
		int envio = t_archivoConfig->MARCOS_SIZE;
		Serializar(MEMORIA, 4, &envio, clienteCpu);

		pthread_create(&hiloAtender, NULL, (void *) atenderCpu, clienteCpu);

		pthread_join(hiloAtender, NULL);
	}

}

int atenderCpu(int socket) {
	while (1) {
		paquete* paqueteRecibido = Deserializar(socket);
		if (paqueteRecibido->header == -1 || paqueteRecibido->header == -2) {
			perror("El chabón se desconectó\n");
			return 1;
		}
		procesar(paqueteRecibido->package, paqueteRecibido->header,
				paqueteRecibido->size, socket);

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
		procesar(paqueteRecibido->package, paqueteRecibido->header,
				paqueteRecibido->size, cliente);

	}
	return 0;
}

void leerComando() {
	while (1) {
		printf("\nIngrese comando\n"
				"1: dump\n"
				"2: buscar frame\n"
				"3: leer de pagina\n"
				"4: escribir en pagina\n"
				"5: size\n");
		scanf("%d", &opcion);
		switch (opcion) {
		case 1: {
			dump();
			break;
		}
		case 2: {
			int32_t pid;
			int32_t pagina;
			printf("ingresar pid\n");
			scanf("%d", &pid);
			printf("ingresar pagina\n");
			scanf("%d", &pagina);
			int32_t unFrame = buscarFrame(pid, pagina);
			printf("el frame correspondiente: ");
			printf("%d\n", unFrame);
			break;
		}
		case 3: {
			int32_t pid;
			int32_t pagina;
			int32_t offset;
			int32_t tamano;
			printf("ingresar pid\n");
			scanf("%d", &pid);
			printf("ingresar pagina\n");
			scanf("%d", &pagina);
			printf("ingresar offset\n");
			scanf("%d", &offset);
			printf("ingresar tamano\n");
			scanf("%d", &tamano);
			char* conten = leerDePagina(pid, pagina, offset, tamano);
			printf("%s/n", conten);
			break;
		}

		case 4: {
			int32_t pid;
			int32_t pagina;
			int32_t offset;
			int32_t tamano;
			char* contenido = malloc(32);
			printf("ingresar pid\n");
			scanf("%d", &pid);
			printf("ingresar pagina\n");
			scanf("%d", &pagina);
			printf("ingresar offset\n");
			scanf("%d", &offset);
			printf("ingresar tamano\n");
			scanf("%d", &tamano);
			printf("ingresar contenido\n");
			scanf("%s", contenido);
			escribirEnPagina(pid, pagina, offset, tamano, contenido);
			break;
		}
		case 5: {
			size();
			break;
		}

		}
	}
}

void procesar(char * paquete, int32_t id, int32_t tamanoPaquete, int32_t socket) {
	int numero_pagina, offset, tamanio, pid_actual;
	switch (id) {
	case ARCHIVO: {
		printf("%s", paquete);
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
		break;
	}
	case TAMANO: {
		int32_t paginas = (int) (*paquete);

		if (paginas > 0) {
			//if (frameGeneral.tamanioDisponible - (paginas*20) >= 0){
			Serializar(OK, 4, &noIMporta, socket);
			//}
		}
		break;
	}
	case PAGINA: {
		int32_t pid;
		printf("%s\n", paquete);
		char *pagina = malloc(t_archivoConfig->MARCOS_SIZE);

		memcpy(pagina, paquete, t_archivoConfig->MARCOS_SIZE);
		pagina[t_archivoConfig->MARCOS_SIZE] = '\0';
		memcpy(&pid, paquete + t_archivoConfig->MARCOS_SIZE, sizeof(int));
		printf("pagina: %s\n", pagina);
		printf("pid: %d\n", pid);
		Serializar(OK, 4, &noIMporta, socket);
		almacernarPaginaEnFrame(pid, tamanoPaquete, paquete);
		break;
	}
	case VARIABLELEER: {
		memcpy(&numero_pagina, paquete, sizeof(int));
		memcpy(&offset, paquete + sizeof(int), sizeof(int));
		memcpy(&tamanio, paquete + sizeof(int) * 2, sizeof(int));
		printf("Quiero leer en la direccion: %d %d %d y le voy a enviar a socket: %d\n",
							numero_pagina, offset,
							offset, socket);
		char * contenido = leerDePagina(1, numero_pagina, offset, tamanio);
		printf("lei: %s\n", contenido);
		Serializar(VARIABLELEER, tamanio, contenido, socket);
		//ojo pid actual
		break;
	}
	case VARIABLEESCRIBIR: {
		memcpy(&numero_pagina, paquete, sizeof(int));
		memcpy(&offset, paquete + sizeof(int), sizeof(int));
		memcpy(&tamanio, paquete + sizeof(int) * 2, sizeof(int));

		buffer = malloc(tamanio);

		memcpy(buffer, paquete + sizeof(int) * 3, tamanio);

		escribirEnPagina(1, numero_pagina, offset, tamanio, buffer);

		int a;
		Serializar(VARIABLELEER, sizeof(int), &a, socket);

		free(buffer);
		break;
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
	frameGeneral.puntero = malloc(frameGeneral.tamanio);
	frameGeneral.punteroDisponible = frameGeneral.puntero;

}

void dump() {
	t_log * log;
	log = log_create("dump.log", "Memoria", 0, LOG_LEVEL_INFO);
	log_info(log, "Tamanio de cache %d", cache1.tamanio);
	log_info(log, "Tamanio disponible de cache %d", cache1.tamanioDisponible);
	int32_t i;
	for (i = 0; i <= 500; i++) {
		if (tablaMemoria[i].pid > 0) {
			log_info(log, "numero de frame %d", i);
			log_info(log, "pid %d", tablaMemoria[i].pid);
			log_info(log, "numero de pagina %d", tablaMemoria[i].numeroPagina);
		}
	}

}

void size() {
	int32_t size;
	printf("size: 0 memoria 1 proceso\n ");
	scanf("%d", &size);
	switch (size) {
	case 0: {
		printf("tamanio total memoria %d\n", frameGeneral.tamanio);
		printf("tamanio disponible memoria %d\n",
				frameGeneral.tamanioDisponible);
		printf("tamanio ocupado memoria %d\n", frameGeneral.tamanioOcupado);
		break;
	}
	case 1: {
		break;
	}

	}
}

/*void crearFrame() {

 frame unFrame;
 int32_t tamanioMarcos, cantidadMarcos;
 cantidadMarcos = t_archivoConfig->MARCOS;
 tamanioMarcos = t_archivoConfig->MARCOS_SIZE;

 unFrame.id = 1;
 unFrame.tamanio = cantidadMarcos * tamanioMarcos;
 unFrame.tamanioDisponible = unFrame.tamanio;
 unFrame.tamanioOcupado = 0;
 unFrame.puntero = malloc(unFrame.tamanio);
 }
 */

void almacernarPaginaEnFrame(int32_t pid, int32_t tamanioBuffer, char* buffer) {
	//SIEMPRE LE TIENE QUE LLEGAR TAMANIO<MARCOS_SIZE OJO
	memcpy(frameGeneral.punteroDisponible, buffer, tamanioBuffer);

	if (pid != pidAnt) {
		numeroPagina = 1;
		pidAnt = pid;
	}
	//nodoTablaMemoria.puntero = frameGeneral.tamanioOcupado;
	nodoTablaMemoria.numeroPagina = numeroPagina;
	frameGeneral.punteroDisponible += t_archivoConfig->MARCOS_SIZE;
	frameGeneral.tamanioOcupado += t_archivoConfig->MARCOS_SIZE;
	frameGeneral.tamanioDisponible -= tamanioBuffer;
	nodoTablaMemoria.pid = pid;

	//memcpy(tablaMemoria[indiceTabla], nodoTablaMemoria, sizeof(nodoTablaMemoria));
	// esta opcion es para usar una tablaMemoria*, el problema es que no se podria
	// accerder a la posicion []
	tablaMemoria[indiceTabla] = nodoTablaMemoria;
	indiceTabla++;
	numeroPagina++;
	//PROBAR

}

int32_t buscarFrame(int32_t pid, int32_t numeroPagina) {
	int32_t i;
	for (i = 0; i <= 500; i++) {
		if (tablaMemoria[i].pid == pid
				&& tablaMemoria[i].numeroPagina == numeroPagina) {
			return i;
		}
	}

	return -1;

}

char* leerDePagina(int32_t pid, int32_t pagina, int32_t offset, int32_t tamano) {

	int32_t unFrame = buscarFrame(pid, pagina);
	char* contenido = malloc(tamano);
	int32_t desplazamiento = unFrame * t_archivoConfig->MARCOS_SIZE + offset;
	memcpy(contenido, frameGeneral.puntero + desplazamiento, tamano);
	contenido[tamano] = '\0';
	return contenido;
}

void escribirEnPagina(int32_t pid, int32_t pagina, int32_t offset,
		int32_t tamano, char* contenido) {

	int32_t unFrame = buscarFrame(pid, pagina);
	int32_t desplazamiento = unFrame * +offset;
	memcpy(frameGeneral.puntero + desplazamiento, contenido, tamano);
}

