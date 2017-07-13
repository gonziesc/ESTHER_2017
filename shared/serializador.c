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
	retorno = recv(socket, &paqueteRecibido->header, sizeof(int),
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

void serializarPCB(programControlBlock *unPcb,int socket, int codigo) {
	int size = 0;
	char *pcbEntero, *pcbConCodigo;
	//char *pcbConEtiquetas;
	size = sizeof(programControlBlock)
			+ unPcb->tamanoIndiceCodigo * 2 * sizeof(int)
			+ unPcb->tamanoindiceEtiquetas * sizeof(char);
	int i, y;
	for (i = 0; i < unPcb->tamanoIndiceStack; i++) {
		size += sizeof(indiceDeStack);
		int y;
		indiceDeStack * stack;
		stack = list_get(unPcb->indiceStack, i);
		for (y = 0; y < stack->tamanoArgs; y++) {
			size += sizeof(posicionMemoria);
		}
		for (y = 0; y < stack->tamanoVars; y++) {

			size += sizeof(variable);

			size += sizeof(posicionMemoria);
		}
	}
	pcbEntero = malloc(size);
	pcbConCodigo = pcbEntero;
	unPcb->tamanoTotal = size;
	memcpy(pcbConCodigo, unPcb, sizeof(programControlBlock));
	pcbConCodigo += sizeof(programControlBlock);
	memcpy(pcbConCodigo, unPcb->indiceCodigo,
			unPcb->tamanoIndiceCodigo * 2 * sizeof(int));
	pcbConCodigo += unPcb->tamanoIndiceCodigo * 2 * sizeof(int);
	memcpy(pcbConCodigo, unPcb->indiceEtiquetas,
			unPcb->tamanoindiceEtiquetas * sizeof(char));
	//pcbConEtiquetas = pcbConCodigo;
	pcbConCodigo += unPcb->tamanoindiceEtiquetas * sizeof(char);
	for (i = 0; i < unPcb->tamanoIndiceStack; i++) {
		indiceDeStack *stack;

		stack = list_get(unPcb->indiceStack, i);
		memcpy(pcbConCodigo, stack, sizeof(indiceDeStack));

		pcbConCodigo += sizeof(indiceDeStack);

		for (y = 0; y < stack->tamanoArgs; y++) {
			posicionMemoria *dir;
			dir = list_get(stack->args, y);
			memcpy(pcbConCodigo, dir, sizeof(posicionMemoria));
			pcbConCodigo += sizeof(posicionMemoria);
		}
		for (y = 0; y < stack->tamanoVars; y++) {
			variable *var;
			posicionMemoria *dir;
			var = list_get(stack->vars, y);
			memcpy(pcbConCodigo, var, sizeof(variable));
			pcbConCodigo += sizeof(variable);
			posicionMemoria * diretemp = var->direccion;
			memcpy(pcbConCodigo, &diretemp->off, 4);
			memcpy(pcbConCodigo + 4, &diretemp->size, 4);
			memcpy(pcbConCodigo + 8, &diretemp->pag, 4);
			pcbConCodigo += sizeof(posicionMemoria);

		}

	}
	Serializar(codigo, unPcb->tamanoTotal, pcbEntero, socket);
}

programControlBlock *deserializarPCB(char *paquete) {
	int size, i, y;
	programControlBlock *pcb;

	pcb = malloc(sizeof(programControlBlock));
	memcpy(pcb, paquete, sizeof(programControlBlock));
	paquete += sizeof(programControlBlock);
	pcb->indiceCodigo = malloc(pcb->tamanoIndiceCodigo * 2 * sizeof(int));
	memcpy(pcb->indiceCodigo, paquete,
			pcb->tamanoIndiceCodigo * 2 * sizeof(int));
	paquete += pcb->tamanoIndiceCodigo * 2 * sizeof(int);
	pcb->indiceEtiquetas = malloc(pcb->tamanoindiceEtiquetas * sizeof(char));
	memcpy(pcb->indiceEtiquetas, paquete,
			pcb->tamanoindiceEtiquetas * sizeof(char));
	paquete += pcb->tamanoindiceEtiquetas * sizeof(char);
	pcb->indiceStack = list_create();
	for (i = 0; i < pcb->tamanoIndiceStack; i++) {
		indiceDeStack *stack = malloc(sizeof(indiceDeStack));
		memcpy(stack, paquete, sizeof(indiceDeStack));
		paquete += sizeof(indiceDeStack);
		stack->vars = list_create();
		stack->args = list_create();
		for (y = 0; y < stack->tamanoArgs; y++) {
			posicionMemoria *dir = malloc(sizeof(posicionMemoria));
			memcpy(dir, paquete, sizeof(posicionMemoria));
			paquete += sizeof(posicionMemoria);
			list_add(stack->args, dir);
		}
		for (y = 0; y < stack->tamanoVars; y++) {
			variable *var = malloc(sizeof(variable));
			posicionMemoria *dire = malloc(sizeof(posicionMemoria));
			memcpy(var, paquete, sizeof(variable));
			paquete += sizeof(variable);

			dire->off = ((int*) (paquete))[0];
			dire->pag = ((int*) (paquete))[2];
			dire->size = ((int*) (paquete))[1];
			var->direccion = dire;
			paquete += sizeof(posicionMemoria);
			list_add(stack->vars, var);
		}
		list_add(pcb->indiceStack, stack);
	}
	return pcb;

}

void destruirCONTEXTO(programControlBlock *pcb) {

	indiceDeStack *stackADestruir;
	while (pcb->tamanoIndiceStack != 0) {
		stackADestruir = list_get(pcb->indiceStack, pcb->tamanoIndiceStack - 1);

		while (stackADestruir->tamanoVars != 0) {

			posicionMemoria*temp = (((variable*) list_get(stackADestruir->vars,
					stackADestruir->tamanoVars - 1))->direccion);
			free(temp);
			free(
					list_get(stackADestruir->vars,
							stackADestruir->tamanoVars - 1));
			stackADestruir->tamanoVars--;
		}
		while (stackADestruir->tamanoArgs != 0) {
			free(
					(posicionMemoria*) list_get(stackADestruir->args,
							stackADestruir->tamanoArgs - 1));
			stackADestruir->tamanoArgs--;
		}
		list_destroy(stackADestruir->vars);

		list_destroy(stackADestruir->args);

		free(list_get(pcb->indiceStack, pcb->tamanoIndiceStack - 1));

		pcb->tamanoIndiceStack--;
	}
	list_destroy(pcb->indiceStack);

	free(pcb->indiceCodigo);

	free(pcb->indiceEtiquetas);

}

void destruirPCB(programControlBlock *pcb) {

	destruirCONTEXTO(pcb);
	free(pcb);

}
