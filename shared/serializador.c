#include "serializador.h"

void Serializar(int32_t id, int32_t tamanio, void* datos, int32_t socket) {
	int tamanio_paquete = 2 * sizeof(int32_t) + tamanio;
	void * buffer = malloc(tamanio_paquete);
	memcpy(buffer, &id, sizeof(id));
	if(tamanio >0){
		memcpy(buffer + sizeof(id), &tamanio, sizeof(id));
		memcpy(buffer + 2 * sizeof(id), datos, tamanio);
	}
	send(socket, buffer, tamanio_paquete, NULL);
	free(buffer);
}

char* Deserializar(int32_t id, int32_t socket, int32_t *tamanio) {
	char* archivoDesempaquetado;
	switch (id) {
	case OK: {

		break;
	}
	case ARCHIVO: {
		if (recv(socket, tamanio, sizeof(id), 0)) {
			archivoDesempaquetado = malloc((*tamanio));
			recv(socket, archivoDesempaquetado, (*tamanio), 0);
			return archivoDesempaquetado;
		}

		break;
	}
	case PAGINA: {
		archivoDesempaquetado = malloc(260);
		recv(socket, archivoDesempaquetado, 260, 0);
		return archivoDesempaquetado;
		break;
	}
	case TAMANO: {
		archivoDesempaquetado = malloc(sizeof(int32_t));
		recv(socket, archivoDesempaquetado, sizeof(int32_t), 0);
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

