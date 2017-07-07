#include "main.h"

unsigned char *mmapDeBitmap;
t_bitarray * bitarray;
archivoConfigFS* t_archivoConfig;
t_config *config;
struct sockaddr_in direccionServidor;
int32_t servidor;
sem_t semConfig;
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
	sem_init(&semConfig, 0, 0);
	configuracion(argv[1], argv[2]);
	sem_wait(&semConfig);
	bitarray = bitarray_create_with_mode(mmapDeBitmap,
			(t_archivoConfig->TAMANIO_BLOQUES
					* t_archivoConfig->CANTIDAD_BLOQUES)
					/ (8 * t_archivoConfig->TAMANIO_BLOQUES), MSB_FIRST);
	printf("El tamano del bitarray es de : %d\n\n\n",
			bitarray_get_max_bit(bitarray));
	idHiloLevantarConexion = pthread_create(&hiloLevantarConexion, NULL,
			levantarConexion, NULL);
	pthread_join(hiloLevantarConexion, NULL);
	return EXIT_SUCCESS;
}

void inicializarMmap() {

	int size;
	char *nombreArchivoRecibido = string_new();
	string_append(&nombreArchivoRecibido, t_archivoConfig->PUERTO_MONTAJE);
	string_append(&nombreArchivoRecibido, "Metadata/Bitmap.bin");
	int bitmap = open(nombreArchivoRecibido, O_RDWR);
	struct stat mystat;

	if (fstat(bitmap, &mystat) < 0) {
		printf("Error al establecer fstat\n");
		close(bitmap);
	}

	/* Get the size of the file. */

	mmapDeBitmap = mmap(NULL, mystat.st_size, PROT_WRITE | PROT_READ,
	MAP_SHARED, bitmap, 0);
	sem_post(&semConfig);
}

void printBitmap() {
	int j;
	for (j = 0; j < 32; j++) {
		bool a = bitarray_test_bit(bitarray, j);
		printf("%i", a);
	}
	printf("\n");
}

void adx_store_data(const char *filepath, const char *data) {
	FILE *fp = fopen(filepath, "w");
	if (fp != NULL) {
		fputs(data, fp);
		fclose(fp);
	}
}

char* obtenerBytesDeUnArchivo(FILE* bloque, int offsetQuePido, int sizeQuePido) {
	char* envio = malloc(sizeQuePido);
	fseek(bloque, offsetQuePido, SEEK_SET);
	fread(envio, sizeQuePido, 1, bloque);
	return envio;

}

char** obtArrayDeBloquesDeArchivo(char* ruta) {
	t_config* configuracion_FS = config_create(ruta);

	return config_get_array_value(configuracion_FS, "BLOQUES");

}

int obtTamanioArchivo(char* ruta) {
	t_config* configuracion_FS = config_create(ruta);

	return config_get_int_value(configuracion_FS, "TAMANIO");

}

void configuracion(char * dir, char* dir2) {
	t_archivoConfig = malloc(sizeof(archivoConfigFS));
	configuracionFS(t_archivoConfig, config, dir, dir2);
	inicializarMmap();

	//printBitmap();
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
		string_append(&nombreArchivoRecibido, "Archivos");
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
		string_append(&nombreArchivoRecibido, "Archivos");
		string_append(&nombreArchivoRecibido, nombreArchivo);
		int j;
		int encontroUnBloque = 0;
		int bloqueEncontrado = 0;
		for (j = 0; j < t_archivoConfig->CANTIDAD_BLOQUES; j++) {

			bool bit = bitarray_test_bit(bitarray, j);
			if (bit == 0) {
				encontroUnBloque = 1;
				bloqueEncontrado = j;
				break;
			}
		}

		if (encontroUnBloque == 1) {
			fp = fopen(nombreArchivoRecibido, "ab+");
			//asignar bloque en el metadata del archivo(y marcarlo como ocupado en el bitmap)
			//escribir el metadata ese del archivo (TAMANO y BLOQUES)

			bitarray_set_bit(bitarray, bloqueEncontrado);

			char *dataAPonerEnFile = string_new();
			string_append(&dataAPonerEnFile, "TAMANIO=");
			string_append(&dataAPonerEnFile, "0");
			string_append(&dataAPonerEnFile, "\n");
			string_append(&dataAPonerEnFile, "BLOQUES=[");
			char* numerito = string_itoa(bloqueEncontrado);
			string_append(&dataAPonerEnFile, numerito);
			string_append(&dataAPonerEnFile, "]");

			adx_store_data(nombreArchivoRecibido, dataAPonerEnFile);

			validado = 1;
			Serializar(CREARARCHIVO, 4, &validado, cliente);
			printf("Se creo el archivo\n");
		} else {
			validado = 0;
			Serializar(CREARARCHIVO, 4, &validado, cliente);
			printf("No se creo el archivo\n");
		}
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
		printf("Data :%s\n", buffer);

		memcpy(nombreArchivo, paquete + 12 + tamanoBuffer, tamanoNombreArchivo);
		strcpy(nombreArchivo + tamanoNombreArchivo, "\0");
		printf("Nombre archivo:%s\n", nombreArchivo);
		char *nombreArchivoRecibido = string_new();
		string_append(&nombreArchivoRecibido, t_archivoConfig->PUERTO_MONTAJE);
		string_append(&nombreArchivoRecibido, "Archivos/");
		string_append(&nombreArchivoRecibido, nombreArchivo);

		printf("Toda la ruta :%s\n", nombreArchivoRecibido);
		if (access(nombreArchivoRecibido, F_OK) != -1) {

			char** arrayBloques = obtArrayDeBloquesDeArchivo(
					nombreArchivoRecibido);

			int d = 0;
			int cantidadBloques = 0;
			while (!(arrayBloques[d] == NULL)) {
				printf("%s \n", arrayBloques[d]);
				d++;
				cantidadBloques++;
			}
			d--;

			printf("Cantidad de bloques :%d\n", cantidadBloques);

			char *nombreBloque = string_new();
			string_append(&nombreBloque, t_archivoConfig->PUERTO_MONTAJE);
			string_append(&nombreBloque, "Bloques/");
			string_append(&nombreBloque, arrayBloques[d]);
			string_append(&nombreBloque, ".bin");
			printf("Nombre del ultimo bloque: %s\n", nombreBloque);
			printf("Tamano del archivo : %d\n",
					(obtTamanioArchivo(nombreArchivoRecibido)));
			printf("Tamano del bloque: %d\n", t_archivoConfig->TAMANIO_BLOQUES);
			int cantRestante = cantidadBloques
					* t_archivoConfig->TAMANIO_BLOQUES
					- ((obtTamanioArchivo(nombreArchivoRecibido)));
			printf("Cantidad restante :%d\n", cantRestante);
			if (tamanoBuffer < cantRestante) {
				adx_store_data(nombreBloque, buffer);
				//send diciendo que todo esta bien
			} else {
				int cuantosBloquesMasNecesito = (tamanoBuffer - cantRestante)
						/ t_archivoConfig->TAMANIO_BLOQUES;

				if (((tamanoBuffer - cantRestante)
						% t_archivoConfig->TAMANIO_BLOQUES) > 0) {
					cuantosBloquesMasNecesito++;
				}
				//si no hay mas bloques de los que se requieren hay que hacer un send tirando error
				int j;
				int r = 0;
				int bloquesEncontrados = 0;
				int bloqs[cuantosBloquesMasNecesito];
				for (j = 0; j < t_archivoConfig->CANTIDAD_BLOQUES; j++) {

					bool bit = bitarray_test_bit(bitarray, j);
					if (bit == 0) {
						if (r == cuantosBloquesMasNecesito) {
							break;
						} else {
							bloqs[r] = j;
							r++;
						}
						bloquesEncontrados++;
					}
				}

				if (bloquesEncontrados >= cuantosBloquesMasNecesito) {
					//guardamos en los bloques deseados

					int s;
					char* loQueVaQuedandoDeBuffer = (char*) buffer;
					for (s = 0; s <= cuantosBloquesMasNecesito; s++) {

						char *nombreBloque = string_new();
						string_append(&nombreBloque,
								t_archivoConfig->PUERTO_MONTAJE);
						string_append(&nombreBloque, "Bloques/");
						char* numerito = string_itoa(bloqs[s]);
						string_append(&nombreBloque, numerito);
						string_append(&nombreBloque, ".bin");
						int offsetAux = 0;
						if (tamanoBuffer > t_archivoConfig->TAMANIO_BLOQUES) {
							//cortar el string

							char* recortado = malloc(
									t_archivoConfig->TAMANIO_BLOQUES);
							memcpy(recortado, buffer + offsetAux,
									t_archivoConfig->TAMANIO_BLOQUES);
							adx_store_data(nombreBloque, recortado);
							offsetAux += t_archivoConfig->TAMANIO_BLOQUES;
							free(recortado);
						} else {
							int sobras = offsetAux % t_archivoConfig->TAMANIO_BLOQUES;
							char* recortado = malloc(
									sobras);
							memcpy(recortado, buffer + offsetAux, sobras);
							//mandarlo to do de una
							adx_store_data(nombreBloque,
									loQueVaQuedandoDeBuffer);
							free(recortado);
						}

						//actualizamos el bitmap
						bitarray_set_bit(bitarray, bloqs[s]);

					}

					//actualizamos el metadata del archivo con los nuevos bloques y el nuevo tamano del archivo

					char *dataAPonerEnFile = string_new();
					string_append(&dataAPonerEnFile, "TAMANIO=");

					int tamanioArchivoViejoInt = obtTamanioArchivo(
							nombreArchivoRecibido);
					int tamanioNuevo = tamanioArchivoViejoInt + (tamanoBuffer);
					char* tamanioNuevoChar = string_itoa(tamanioNuevo);
					string_append(&dataAPonerEnFile, tamanioNuevoChar);
					string_append(&dataAPonerEnFile, "\n");
					string_append(&dataAPonerEnFile, "BLOQUES=[");
					int z;

					char** arrayBloques = obtArrayDeBloquesDeArchivo(
							nombreArchivoRecibido);
					int d = 0;
					while (!(arrayBloques[d] == NULL)) {
						string_append(&dataAPonerEnFile, arrayBloques[d]);
						string_append(&dataAPonerEnFile, ",");
						d++;
					}
					for (z = 0; z < cuantosBloquesMasNecesito; z++) {
						char* bloqueString = string_itoa(bloqs[z]);
						string_append(&dataAPonerEnFile, bloqueString);
						if (!(z == (cuantosBloquesMasNecesito - 1))) {
							string_append(&dataAPonerEnFile, ",");
						}
					}

					string_append(&dataAPonerEnFile, "]");
					fclose(fopen(nombreArchivoRecibido, "w"));
					adx_store_data(nombreArchivoRecibido, dataAPonerEnFile);

					validado = 1;

					Serializar(GUARDARDATOS, 4, &validado, cliente);

				} else {
					validado = 0;

					Serializar(GUARDARDATOS, 4, &validado, cliente);
				}

			}
			validado = 1;
			Serializar(GUARDARDATOS, 4, &validado, cliente);

		} else {
			validado = 0;

			//MANDAR QUE NO EXISTE EL ARCHIVO?
		}
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
		if (access(nombreArchivoRecibido, F_OK) != -1) {

			int calcularBloqueInicial = offset
					/ t_archivoConfig->TAMANIO_BLOQUES;
			int offsetInicial = offset % t_archivoConfig->TAMANIO_BLOQUES;
			int indice = calcularBloqueInicial;
			fp = fopen(nombreArchivoRecibido, "r");
			char** arrayBloques = obtArrayDeBloquesDeArchivo(
					nombreArchivoRecibido);
			int cantidadBloquesQueNecesito = size
					/ t_archivoConfig->TAMANIO_BLOQUES;
			if ((size % t_archivoConfig->TAMANIO_BLOQUES) != 0) {
				cantidadBloquesQueNecesito++;
			}

			char* infoTraidaDeLosArchivos = string_new();
			int cantidadDeBloquesAux = cantidadBloquesQueNecesito;
			int sizeFaltante = size;
			while (!(arrayBloques[indice] == NULL || sizeFaltante == 0)) {

				int nroBloque = atoi(arrayBloques[indice]);
				char *nombreBloque = string_new();
				string_append(&nombreBloque, t_archivoConfig->PUERTO_MONTAJE);
				string_append(&nombreBloque, "Bloques/");
				string_append(&nombreBloque, arrayBloques[indice]);
				string_append(&nombreBloque, ".bin");

				FILE *bloque = fopen(nombreBloque, "r");
				int sizeQuePido;
				if (sizeFaltante > t_archivoConfig->TAMANIO_BLOQUES) {
					sizeQuePido = t_archivoConfig->TAMANIO_BLOQUES;
					sizeFaltante -= t_archivoConfig->TAMANIO_BLOQUES;
				} else {
					sizeQuePido = sizeFaltante;
					sizeFaltante = 0;
				}
				int offsetQuePido = offsetInicial;
				char* data = obtenerBytesDeUnArchivo(bloque, offsetQuePido,
						sizeQuePido);
				string_append(&infoTraidaDeLosArchivos, data);
				free(data);
				indice++;
			}

			validado = 1;

			int tamanoAMandar = sizeof(int) * strlen(infoTraidaDeLosArchivos);
			void * envio = malloc(tamanoAMandar + 4);
			memcpy(envio, &tamanoAMandar, 4);
			memcpy(envio + 4, infoTraidaDeLosArchivos, tamanoAMandar);

			Serializar(OBTENERDATOS, tamanoAMandar + 4, envio, cliente);
			free(envio);

		} else {
			validado = 0;
			int tamanoAMandar = -1;
			void * envio = malloc(8 + 4);
			memcpy(envio, &tamanoAMandar, 4);
			memcpy(envio + 4, &validado, 4);

			Serializar(OBTENERDATOS, 8, envio, cliente);
			free(envio);
		}

		break;
	}
	case BORRARARCHIVOFS: {
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
		string_append(&nombreArchivoRecibido, "Archivos");
		string_append(&nombreArchivoRecibido, nombreArchivo);

		if (access(nombreArchivoRecibido, F_OK) != -1) {

			fp = fopen(nombreArchivoRecibido, "w");
			//poner en un array los bloques de ese archivo para luego liberarlos
			char** arrayBloques = obtArrayDeBloquesDeArchivo(
					nombreArchivoRecibido);

			if (remove(nombreArchivoRecibido) == 0) {

				validado = 1;

				//marcar los bloques como libres dentro del bitmap (recorriendo con un for el array que cree arriba)
				int d = 0;
				while (!(arrayBloques[d] == NULL)) {
					int indice = atoi(arrayBloques[d]);
					bitarray_clean_bit(bitarray, indice);
					d++;
				}
				validado = 1;
				Serializar(BORRARARCHIVOFS, 4, &validado, cliente);

			} else {
				validado = 0;
				Serializar(BORRARARCHIVOFS, 4, &validado, cliente);
			}
		} else {
			validado = 0;
			Serializar(BORRARARCHIVOFS, 4, &validado, cliente);
		}
		break;
	}

	}
}
