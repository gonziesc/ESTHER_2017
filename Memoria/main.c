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

typedef struct {
	int32_t id;
	int32_t tamanio;
	int32_t tamanioDisponible;
	int32_t tamanioOcupado;
	char* puntero;
} frame;
frame frameGeneral;

typedef struct {
	int32_t pid;
	int32_t puntero;
} infoTablaMemoria;

infoTablaMemoria nodoTablaMemoria;
infoTablaMemoria* tablaMemoria;

pthread_t hiloLevantarConexion;
int32_t idHiloLevantarConexion;

pthread_t hiloCpu;
int32_t idHiloCpu;

int32_t main(int argc, char**argv) {

	printf("memoria \n");
	configuracion(argv[1]);

	idHiloLevantarConexion = pthread_create(&hiloLevantarConexion, NULL,
			levantarConexion, NULL);
	pthread_join(hiloLevantarConexion, NULL);
	crearFrame();
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
	Serializar(MEMORIA, 4, 0, cliente);

	while (1) {
		int32_t bytesRecibidos = recv(cliente, &header, 4, 0);
		if (bytesRecibidos <= 0) {
			perror("El chabón se desconectó\n");
			return 1;
		}
		char* paquete = Deserializar(header, cliente, tamanoPaquete);
		procesar(paquete, header, tamanoPaquete, cliente);
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

	}
	case TAMANO: {
		int paginas = (int) (*paquete);
		if (paginas > 0) { // validar que haya la cantidad de paginas disp
			Serializar(OK, 4, 0, socket);
		}
	}
	}
}

void crearFrame() {

	int32_t tamanioMarcos, cantidadMarcos;
	cantidadMarcos = t_archivoConfig->MARCOS;
	tamanioMarcos = t_archivoConfig->MARCOS_SIZE;

	frameGeneral.id = 1;
	frameGeneral.tamanio = cantidadMarcos * tamanioMarcos;
	frameGeneral.tamanioDisponible = frameGeneral.tamanio;
	frameGeneral.tamanioOcupado = 0;
	frameGeneral.puntero = malloc(frameGeneral.tamanio);
}

void almacernarArchivo(char* archivo, int32_t tamanioArchivo, int32_t pid) {
	if (frameGeneral.tamanioDisponible - tamanioArchivo >= 0) {
		//frameGeneral.tamanioDisponible -= tamanioArchivo;
		//frameGeneral.puntero[frameGeneral.tamanioOcupado]= strcpy(frameGeneral.puntero,archivo);
		// o memcpy? memcpy(frameGeneral.puntero,archivo, tamanioArchivo);

		memcpy(frameGeneral.puntero, archivo, tamanioArchivo);
		frameGeneral.tamanioOcupado += tamanioArchivo;
		nodoTablaMemoria.pid = pid;
		nodoTablaMemoria.puntero = frameGeneral.tamanioOcupado;
		int32_t indiceTabla = 1;
		tablaMemoria[indiceTabla] = nodoTablaMemoria;
		indiceTabla++;

		//PROBAR

	} else
		printf(
				"ERROR: no alcanza el tamanio en memoria para guadrar el archivo");

}

