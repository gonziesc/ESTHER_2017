#include "serializador.h"

void Serializar(int32_t id, int32_t tamanio, void* datos, int32_t socket) {
	int tamanio_paquete = 2 * sizeof(int32_t) + tamanio;
	void * buffer = malloc(tamanio_paquete);
	memcpy(buffer, &id, sizeof(id));
	memcpy(buffer + sizeof(id), &tamanio, sizeof(id));
	memcpy(buffer + 2 * sizeof(id), datos, tamanio);
	send(socket, buffer, tamanio_paquete, NULL);
	free(buffer);
}

paquete* Deserializar(int32_t socket) {
	paquete * paqueteRecibido = malloc(sizeof(paquete));
	int retorno = 0;
	retorno = recv(socket, &paqueteRecibido->header,
			sizeof(int),
			NULL);

	if (retorno == 0) {
		paqueteRecibido->header = -1;
		void * informacion_recibida = malloc(sizeof(int));
		paqueteRecibido->package = informacion_recibida;
		return paqueteRecibido;

	}
	if (retorno < 0) {
			paqueteRecibido->header = -2;
			void * informacion_recibida = malloc(sizeof(int));
			paqueteRecibido->package = informacion_recibida;
			return paqueteRecibido;

		}
	recv(socket, &paqueteRecibido->size, sizeof(int),
			NULL);

	void * informacion_recibida = malloc(paqueteRecibido->size);

	recv(socket, informacion_recibida, paqueteRecibido->size,
			NULL);

	paqueteRecibido->package = informacion_recibida;

	return paqueteRecibido;

}



