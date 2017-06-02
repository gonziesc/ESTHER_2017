#include "main.h"

int32_t opcion;
char nombreArchivo[100];
archivoConfigConsola* t_archivoConfig;
t_config *config;
struct sockaddr_in direccionKernel;
direccionMem;
int32_t cliente;
procesosActualesPosicion = 0;
header;
tamanoPaquete;
buffer;
bytesRecibidos;
idHiloLeerComando;
idHiloConectarseConKernel;
pthread_t hiloLeerComando;
hiloConectarseConKernel;
ProcesosActuales procesosActuales[100];
int noInteresa;

int32_t main(int argc, char**argv) {
	Configuracion(argv[1]);
	idHiloConectarseConKernel = pthread_create(&hiloConectarseConKernel, NULL,
			ConectarseConKernel, noInteresa);
	idHiloLeerComando = pthread_create(&hiloLeerComando, NULL, leerComando,
	NULL);
	pthread_join(hiloConectarseConKernel, NULL);
	pthread_join(hiloLeerComando, NULL);
	return EXIT_SUCCESS;
}
void Configuracion(char* dir) {
	t_archivoConfig = malloc(sizeof(archivoConfigConsola));
	configuracionConsola(t_archivoConfig, config, dir);
}
int32_t ConectarseConKernel(int noIMporta) {
	llenarSocketAdrrConIp(&direccionKernel, t_archivoConfig->IP_KERNEL,
			t_archivoConfig->PUERTO_KERNEL);
	cliente = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(cliente, (void*) &direccionKernel, sizeof(direccionKernel))
			!= 0) {
		perror("No se pudo conectar");
		return 1;
	}

	//Serializar(CONSOLA, 4, noIMporta, cliente);
	//VER POR QUE MIERDA NO ANDA ESTO

}

void leerComando() {
	while (1) {
		printf("Ingrese comando\n");
		printf("1: iniciar programa\n");
		scanf("%d", &opcion);
		switch (opcion) {
		case 1: {
			pthread_t hiloPrograma;
			int32_t idHiloId;
			idHiloId = pthread_create(&hiloPrograma, NULL, crearNuevoProceso,
					procesosActualesPosicion);
			procesosActuales[procesosActualesPosicion].identificadorHilo =
					idHiloId;
			procesosActualesPosicion++;
			pthread_join(hiloPrograma, NULL);
			break;
		}
		}
	}
}

void crearNuevoProceso(int procesosActualesPosicion) {
	printf("Ingrese la ruta del archivo\n");
	scanf("%s", &nombreArchivo);
	char *contenidoDelArchivo = malloc(1000);
	int tamano = abrirYLeerArchivo(nombreArchivo, contenidoDelArchivo);
	Serializar(ARCHIVO, tamano, contenidoDelArchivo, cliente);
	paquete* paqueteRecibido = Deserializar(cliente);
	if (paqueteRecibido->header == PID) {
		int processID;
		memcpy(&processID, paqueteRecibido->package, 4);
		procesosActuales[procesosActualesPosicion].PID = processID;
		printf("process id: %d", processID);
	}

}

int abrirYLeerArchivo(char path[], char* string) {

	FILE *f = fopen(path, "rb");
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);  //same as rewind(f);
	fread(string, fsize, 1, f);
	fclose(f);
	return fsize;
}
