#include "serializador.h"
typedef enum {
	ok = 0,
	archivo = 1,
	pcb = 2,
	memoria = 3,
	fs = 4,
	kernel = 5,
	cpu = 6,
	consola = 7,
	codigo = 8
} codigosSerializador;

void Serializar(int32_t id, int32_t tamanioArchivo, char* buffer,
		char* archivoEmpaquetado) {
	switch (id) {
	case ok: {
		memcpy(archivoEmpaquetado, id, 4);
		memcpy(archivoEmpaquetado + 4, tamanioArchivo, 4);
		break;
	}
	case archivo: {
		memcpy(archivoEmpaquetado, &id, sizeof(id));
		memcpy(archivoEmpaquetado + 4, &tamanioArchivo, sizeof(tamanioArchivo));
		memcpy(archivoEmpaquetado + 8, buffer, tamanioArchivo);
		break;
	}
	case pcb: {

		break;
	}

	case fs: {
		break;
	}
	case kernel: {
		break;
	}
	case cpu: {
		break;
	}
	case consola: {
		break;
	}
	case codigo: {

	}
	}
}

char* Deserializar(int32_t id, int32_t socket) {
	char* archivoDesempaquetado;
	switch (id) {
	case ok: {
		printf("Ok");
		archivoDesempaquetado = 0;
		break;
	}
	case archivo: {
		int32_t tamanio;
		if (recv(socket, &tamanio, 4, 0)) {
			//archivoDesempaquetado = malloc(tamanio + 1);
			int comparacion = 0;
			archivoDesempaquetado = string_new();
			char *datos_tmp = malloc(tamanio + 1);
			memset(datos_tmp, '\0', tamanio + 1);
			while (comparacion != tamanio) {
				comparacion += recv(socket, datos_tmp, tamanio - comparacion,
						0);
				string_append(&archivoDesempaquetado, datos_tmp);
				memset(datos_tmp, '\0', tamanio + 1);
			}
			printf("%s\n", archivoDesempaquetado);
			return archivoDesempaquetado;
			//int comparacion = 0;
			/*archivoDesempaquetado = string_new();
			 char* datos_tmp = malloc(tamanio + 1);
			 memset(datos_tmp, '\0', tamanio + 1);
			 while (comparacion != tamanio) {
			 comparacion += recv(socket, datos_tmp,
			 tamanio - comparacion, 0);
			 string_append(&archivoDesempaquetado, datos_tmp);
			 memset(datos_tmp, '\0', tamanio + 1);*/

		}

		//memset(archivoDesempaquetado,'\0',tamanio+1);
		//recv(socket,&archivoDesempaquetado , tamanio, 0);
		//printf("%s", archivoDesempaquetado);
		//free(archivoDesempaquetado);

		break;
	}
	case pcb: {

		break;
	}

	case fs: {
		break;
	}
	case kernel: {
		break;
	}
	case cpu: {
		break;
	}
	case consola: {
		break;
	}
	case codigo: {

	}
		return archivoDesempaquetado;

	}
}

