#include "main.h"
archivoConfigMemoria* t_archivoConfig;
t_config *config;
struct sockaddr_in direccionServidor;
int32_t servidor;
int32_t activado;
int32_t cliente;
int32_t header;
struct sockaddr_in direccionCliente;
uint32_t tamanoDireccion;
char* buffer;
int32_t tamanoPaquete;
int32_t opcion;
cache cache1;

frame frameGeneral;

infoTablaMemoria tablaMemoria[500];
int32_t indiceTabla = 0;
infoTablaMemoria nodoTablaMemoria;



pthread_t hiloLevantarConexion;
int32_t idHiloLevantarConexion;

pthread_t hiloCpu;
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

	cliente = accept(servidor, (void*) &direccionCliente, &tamanoDireccion);
	printf("Recibí una conexión en %d!!\n", cliente);
	int envio = t_archivoConfig->MARCOS_SIZE;
	Serializar(MEMORIA, 4, &envio , cliente);

	while (1) {
		paquete* paqueteRecibido = Deserializar(cliente);
		if (paqueteRecibido->header == -1  ||paqueteRecibido->header == -2) {
			perror("El chabón se desconectó\n");
			return 1;
		}
		procesar(paqueteRecibido->package, paqueteRecibido->header, paqueteRecibido->size, cliente);

	}
}


void leerComando() {
	while (1) {
		printf("Ingrese comando\n"
				"1: dump\n");
		scanf("%d", &opcion);
		switch (opcion) {
		case 1: {
				dump();
			break;
		}
		}
	}
}




void procesar(char * paquete, int32_t id, int32_t tamanoPaquete, int32_t socket) {
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
		// levantar hilo por cada cpu que le llega con el socket para enviar y recibir de ese cpu
		//tamanio, paquete, header, socket
		//idhiloCpu = pthread_create(&hiloCpu, NULL, , NULL);
		//pthread_join(hiloCpu, NULL);

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
			int noIMporta;
			Serializar(OK, 4, noIMporta, socket);
			//}
		}
		break;
	}
	case PAGINA: {
		int32_t pid;
		printf("%s\n", paquete);
		char *pagina = malloc( t_archivoConfig->MARCOS_SIZE);

		memcpy(pagina, paquete, t_archivoConfig->MARCOS_SIZE);
		pagina[t_archivoConfig->MARCOS_SIZE] = '\0';
		memcpy(&pid, paquete +  t_archivoConfig->MARCOS_SIZE, sizeof(int));
		printf("pagina: %s\n", pagina);
		printf("pid: %d\n", pid);
		int noIMporta;
		Serializar(OK, 4, noIMporta, socket);
		almacernarPaginaEnFrame(pid,  tamanoPaquete,  paquete);


	}
	}

}

void crearFrameGeneral() {


	int32_t tamanioMarcos, cantidadMarcos;
	cantidadMarcos = t_archivoConfig->MARCOS;
	tamanioMarcos = t_archivoConfig->MARCOS_SIZE;

	frameGeneral.id = 1;
	frameGeneral.tamanio = cantidadMarcos * tamanioMarcos;
	frameGeneral.tamanioDisponible = frameGeneral.tamanio;
	frameGeneral.tamanioOcupado = 0;
	frameGeneral.puntero = malloc(frameGeneral.tamanio);

}
void dump(){
	t_log * log;
	log = log_create("dump.log", "Memoria", 0, LOG_LEVEL_INFO);
	log_info(log, "Tamanio de cache", cache1.tamanio);
	log_info(log, "Tamanio disponible de cache", cache1.tamanioDisponible);
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


		memcpy(frameGeneral.puntero, buffer, tamanioBuffer);


		nodoTablaMemoria.puntero = frameGeneral.tamanioOcupado;
		frameGeneral.tamanioOcupado += tamanioBuffer;
		nodoTablaMemoria.pid = pid;


		//memcpy(tablaMemoria[indiceTabla], nodoTablaMemoria, sizeof(nodoTablaMemoria));
		// esta opcion es para usar una tablaMemoria*, el problema es que no se podria
		// accerder a la posicion []
		tablaMemoria[indiceTabla] = nodoTablaMemoria;
		indiceTabla++;
		//PROBAR


}

