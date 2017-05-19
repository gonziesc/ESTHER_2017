#include "serializador.h"

void Serializar(int32_t id, int32_t tamanioArchivo, char* buffer,
		int32_t socket) {
	char* archivoEmpaquetado;
	switch (id) {
	case OK:
	case CONSOLA:
	case CPU:
	case KERNEL:
	case FILESYSTEM:
	case MEMORIA: {
		archivoEmpaquetado = malloc(4);
		memcpy(archivoEmpaquetado, &id, sizeof(id));
		send(socket, archivoEmpaquetado, sizeof(id), 0);
		free(archivoEmpaquetado);
		break;
	}
	case TAMANO: {
		archivoEmpaquetado = malloc(8);
		memcpy(archivoEmpaquetado, &id, sizeof(id));
		memcpy(archivoEmpaquetado + 4, buffer, sizeof(id));
		send(socket, archivoEmpaquetado, 8, 0);
		free(archivoEmpaquetado);
		break;
	}
	case ARCHIVO: {
		archivoEmpaquetado = malloc(8 + tamanioArchivo);
		memcpy(archivoEmpaquetado, &id, sizeof(id));
		memcpy(archivoEmpaquetado + 4, &tamanioArchivo, sizeof(tamanioArchivo));
		memcpy(archivoEmpaquetado + 8, buffer, tamanioArchivo);
		send(socket, archivoEmpaquetado, tamanioArchivo + 8, 0);
		free(archivoEmpaquetado);
		break;
	}
	case PCB: {

		break;
	}
	case CODIGO: {

	}
	}
}

char* Deserializar(int32_t id, int32_t socket, int32_t *tamanio) {
	char* archivoDesempaquetado;
	switch (id) {
	case OK: {

		break;
	}
	case ARCHIVO: {
		if (recv(socket, tamanio, 4, 0)) {
			archivoDesempaquetado = malloc((*tamanio));
			recv(socket, archivoDesempaquetado, (*tamanio), 0);
			//printf("%s\n", archivoDesempaquetado);
			return archivoDesempaquetado;
		}

		break;
	}
	case TAMANO: {
			archivoDesempaquetado = malloc(4);
			recv(socket, archivoDesempaquetado, 4, 0);
			return archivoDesempaquetado;


			break;
		}
	case PCB: {

		break;
	}

	case FILESYSTEM: {

		break;
	}
	case KERNEL: {

		break;
	}
	case CPU: {

		break;
	}
	case CONSOLA: {

		break;
	}
	case MEMORIA: {

		break;
	}
	case CODIGO: {

	}

	}
	//DEBUGUEAR MEMORIA
	return archivoDesempaquetado;

}

