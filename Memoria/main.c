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


typedef struct {
	int32_t pid;
	int32_t puntero;
} infoTablaMemoria;

typedef struct{
	int32_t tamanio;
	int32_t tamanioDisponible;
}cache;

cache cache1;

frame frameGeneral;

infoTablaMemoria tablaMemoria[10];
int32_t indiceTabla = 1;
infoTablaMemoria nodoTablaMemoria;



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
	crearFrameGeneral();
	//dump();
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
	Serializar(MEMORIA, 0, 0, cliente);

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
		break;
	}
	case TAMANO: {
		int32_t paginas = (int) (*paquete);

		if (paginas > 0) {
			if (frameGeneral.tamanioDisponible - tamanoPaquete >= 0){
			Serializar(OK, 0, 0, socket);
			}
		}
		break;
	}
	case PAGINA: {
		int32_t pid;
		printf("%s\n", paquete);
		char *pagina = malloc(20);
		//Te llego pagina y pid. con pagina, lo que haces es memcpy(framegigante, pagina, 256)
		//asignar char* a framegigante + 0
		memcpy(pagina, paquete, 20);
		memcpy(&pid, pagina + 20, 4);
		printf("%pagina: s\n", pagina);
		printf("%pid: d\n", pid);
		Serializar(OK, 0, 0, socket);
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

	FILE* archivoDump = fopen("dump.txt","rb+");
	//int a = fwrite(&cache1, sizeof(cache), 1, archivoDump);

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

