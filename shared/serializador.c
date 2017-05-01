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
		 int32_t socket) {
	char* archivoEmpaquetado;
	switch (id) {
	case ok:
	case consola:
	case cpu:
	case kernel:
	case fs:
	case memoria: {
		archivoEmpaquetado = malloc(4);
		memcpy(archivoEmpaquetado, &id, sizeof(id));
		send(socket, archivoEmpaquetado, sizeof(id), 0);
		free(archivoEmpaquetado);
		break;
	}
	case archivo: {
		archivoEmpaquetado = malloc(8 + tamanioArchivo);
		memcpy(archivoEmpaquetado, &id, sizeof(id));
		memcpy(archivoEmpaquetado + 4, &tamanioArchivo, sizeof(tamanioArchivo));
		memcpy(archivoEmpaquetado + 8, buffer, tamanioArchivo);
		send(socket, archivoEmpaquetado, tamanioArchivo +8, 0);
		free(archivoEmpaquetado);
		break;
	}
	case pcb: {

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
		break;
	}
	case archivo: {
		int32_t tamanio;
		if (recv(socket, &tamanio, 4, 0)) {
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

		}

		break;
	}
	case pcb: {

		break;
	}

	case fs: {
		printf("Se conecto FS");
		break;
	}
	case kernel: {
		printf("Se conecto Kernel");
		break;
	}
	case cpu: {
		printf("Se conecto CPU");
		break;
	}
	case consola: {
		printf("Se conecto Consola");
		break;
	}
	case memoria: {
		printf("Se conecto memoria");
		break;
	}
	case codigo: {

	}
		return archivoDesempaquetado;

	}
}

