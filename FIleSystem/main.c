#include "main.h"
archivoConfigFS* t_archivoConfig;
t_config *config;
struct sockaddr_in direccionServidor;
int32_t servidor;
int32_t activado;
int32_t header;
struct sockaddr_in direccionCliente;
uint32_t tamanoDireccion;
int32_t cliente;
int32_t tamanoPaquete;
char* buffer;
pthread_t hiloLevantarConexion;
int32_t idHiloLevantarConexion;
int noInteresa;

int32_t main(int argc, char**argv) {
	configuracion(argv[1]);
	idHiloLevantarConexion = pthread_create(&hiloLevantarConexion, NULL,
			levantarConexion, NULL);
	pthread_join(hiloLevantarConexion, NULL);
	return EXIT_SUCCESS;
}
void configuracion(char * dir) {
	t_archivoConfig = malloc(sizeof(archivoConfigFS));
	configuracionFS(t_archivoConfig, config, dir);
}
int32_t levantarConexion() {
	llenarSocketAdrr(&direccionServidor, t_archivoConfig->PUERTO_KERNEL);
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
	Serializar(FILESYSTEM, 4, &noInteresa, cliente);
	printf("Recibí una conexión en %d!!\n", cliente);
	while (1) {
		paquete* paqueteRecibido = Deserializar(cliente);
		if (paqueteRecibido->header < 0) {
			perror("Kernel se desconectó\n");
			return 1;
		}

		procesar(paqueteRecibido->package, paqueteRecibido->header,
				paqueteRecibido->size);
	}
}

void procesar(char * paquete, int32_t id, int32_t tamanoPaquete) {
	switch (id) {
	case KERNEL: {
		printf("Se conecto Kernel\n");
		break;
	}
	case VALIDARARCHIVO: {
		int tamanoArchivo;
		int validado;

		memcpy(&tamanoArchivo, paquete, sizeof(int));
		char* nombreArchivo = malloc(
				tamanoArchivo * sizeof(char) + sizeof(char));
		memcpy(nombreArchivo, paquete + 4, tamanoArchivo);
		strcpy(nombreArchivo + tamanoArchivo, "\0");

		char *nombreArchivoRecibido = string_new();
		string_append(&nombreArchivoRecibido, t_archivoConfig->PUERTO_MONTAJE);
		string_append(&nombreArchivoRecibido, "Archivos/");
		string_append(&nombreArchivoRecibido, nombreArchivo);
		printf("%s\n", nombreArchivoRecibido);
		if (access(nombreArchivoRecibido, F_OK) != -1) {
			// file exists
			validado = 1;
			Serializar(VALIDARARCHIVO, 4, &validado, cliente);
		} else {
			// file doesn't exist
			validado = 0;
			Serializar(VALIDARARCHIVO, 4, &validado, cliente);
		}
		break;
	}
	case CREARARCHIVO: {
		FILE *fp;

		int tamanoArchivo;
		int validado;

		memcpy(&tamanoArchivo, paquete, sizeof(int));
		char* nombreArchivo = malloc(
				tamanoArchivo * sizeof(char) + sizeof(char));
		memcpy(nombreArchivo, paquete + 4, tamanoArchivo);
		strcpy(nombreArchivo + tamanoArchivo, "\0");

		char *nombreArchivoRecibido = string_new();
		string_append(&nombreArchivoRecibido, t_archivoConfig->PUERTO_MONTAJE);
		string_append(&nombreArchivoRecibido, "Archivos/");
		string_append(&nombreArchivoRecibido, nombreArchivo);
		break;
	}
	case GUARDARDATOS: {
		FILE *fp;

		int tamanoNombreArchivo;
		int validado;
		int puntero;
		int tamanoBuffer;

		memcpy(&tamanoNombreArchivo, paquete, sizeof(int));
		printf("Tamano nombre archivo:%d\n", tamanoNombreArchivo);
		char* nombreArchivo = malloc(tamanoNombreArchivo);

		memcpy(&puntero, paquete + 4, sizeof(int));
		printf("Puntero:%d\n", puntero);

		memcpy(&tamanoBuffer, paquete + 8, sizeof(int));
		printf("Tamano de la data:%d\n", tamanoBuffer);
		char* buffer = malloc(tamanoBuffer);

		memcpy(buffer, paquete + 12, tamanoBuffer);
		strcpy(buffer + tamanoBuffer, "\0");
		printf("Data :%s\n", buffer);

		memcpy(nombreArchivo, paquete + 12 + tamanoBuffer, tamanoNombreArchivo);
		strcpy(nombreArchivo + tamanoNombreArchivo, "\0");
		printf("Nombre archivo:%s\n", nombreArchivo);
		char *nombreArchivoRecibido = string_new();
		string_append(&nombreArchivoRecibido, t_archivoConfig->PUERTO_MONTAJE);
		string_append(&nombreArchivoRecibido, "Archivos/");
		string_append(&nombreArchivoRecibido, nombreArchivo);

		printf("Toda la ruta :%s\n", nombreArchivoRecibido);
		break;
	}
	case OBTENERDATOS: {
		FILE *fp;

		int tamanoArchivo;
		int validado;
		int offset;
		int size;

		memcpy(&tamanoArchivo, paquete, sizeof(int));
		memcpy(&size, paquete + 4, sizeof(int));
		memcpy(&offset, paquete + 8, sizeof(int));
		char* nombreArchivo = malloc(
				tamanoArchivo * sizeof(char) + sizeof(char));
		memcpy(nombreArchivo, paquete + 12, tamanoArchivo);
		strcpy(nombreArchivo + tamanoArchivo, "\0");

		char *nombreArchivoRecibido = string_new();
		string_append(&nombreArchivoRecibido, t_archivoConfig->PUERTO_MONTAJE);
		string_append(&nombreArchivoRecibido, "Archivos/");
		string_append(&nombreArchivoRecibido, nombreArchivo);
		break;
	}
	case BORRARARCHIVO: {
		FILE *fp;

		int tamanoArchivo;
		int validado;

		memcpy(&tamanoArchivo, paquete, sizeof(int));
		char* nombreArchivo = malloc(
				tamanoArchivo * sizeof(char) + sizeof(char));
		memcpy(nombreArchivo, paquete + 4, tamanoArchivo);
		strcpy(nombreArchivo + tamanoArchivo, "\0");

		char *nombreArchivoRecibido = string_new();
		string_append(&nombreArchivoRecibido, t_archivoConfig->PUERTO_MONTAJE);
		string_append(&nombreArchivoRecibido, "Archivos/");
		string_append(&nombreArchivoRecibido, nombreArchivo);
		break;
	}

	}
}
