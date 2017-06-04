#include "main.h"

archivoConfigCPU* t_archivoConfig;
t_config *config;
struct sockaddr_in direccionKernel;
int32_t cliente;
char* buffer;
struct sockaddr_in direccionMem;
int32_t clienteMEM;
int32_t bytesRecibidos;
int32_t header;
int32_t tamanoPaquete;
programControlBlock *pcb;
int32_t tamanoPag;
pthread_t hiloKernel;
pthread_t hiloMemoria;
pthread_mutex_t mutexProcesar;
sem_t semProcesar;
int noInteresa;
AnSISOP_funciones primitivas = { .AnSISOP_definirVariable =
		dummy_definirVariable, .AnSISOP_obtenerPosicionVariable =
		dummy_obtenerPosicionVariable, .AnSISOP_dereferenciar =
		dummy_dereferenciar, .AnSISOP_asignar = dummy_asignar,
		.AnSISOP_finalizar = dummy_finalizar,

};

int32_t main(int argc, char**argv) {
	Configuracion(argv[1]);
	pthread_create(&hiloKernel, NULL, ConectarConKernel, NULL);
	pthread_create(&hiloMemoria, NULL, conectarConMemoria, NULL);

	/*char* sentencia = "begin";
	 analizadorLinea(depurarSentencia(sentencia), &primitivas, NULL);
	 char* sentencia = "variables a, b";
	 analizadorLinea(depurarSentencia(sentencia), &primitivas, NULL);
	 sentencia = "a = 3";
	 analizadorLinea(depurarSentencia(sentencia), &primitivas, NULL);
	 sentencia = "b = 5";
	 analizadorLinea(depurarSentencia(sentencia), &primitivas, NULL);
	 sentencia = "a = b + 12";
	 analizadorLinea(depurarSentencia(sentencia), &primitivas, NULL);
	 sentencia = "end";
	 analizadorLinea(depurarSentencia(sentencia), &primitivas, NULL);
	 */
	pthread_join(hiloKernel, NULL);
	pthread_join(hiloMemoria, NULL);
	return EXIT_SUCCESS;
}
void Configuracion(char* dir) {
	t_archivoConfig = malloc(sizeof(archivoConfigCPU));
	configuracionCpu(t_archivoConfig, config, dir);
	sem_init(&semProcesar, 0, 1);
}

int32_t conectarConMemoria() {
	llenarSocketAdrrConIp(&direccionMem, t_archivoConfig->IP_MEMORIA,
			t_archivoConfig->PUERTO_MEMORIA);
	clienteMEM = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(clienteMEM, (void*) &direccionMem, sizeof(direccionMem)) != 0) {
		perror("No se pudo conectar");
		return 1;
	}
	Serializar(CPU, 4, &noInteresa, clienteMEM);
	while (1) {
		sem_wait(&semProcesar);
		paquete* paqueteRecibido = Deserializar(clienteMEM);
		if (paqueteRecibido->header < 0) {
			perror("Memoria se desconectó");
			return 1;
		}
		pthread_mutex_lock(&mutexProcesar);
		procesar(paqueteRecibido->package, paqueteRecibido->header,
				tamanoPaquete);
		pthread_mutex_unlock(&mutexProcesar);
	}
}

int32_t ConectarConKernel() {
	llenarSocketAdrrConIp(&direccionKernel, t_archivoConfig->IP_KERNEL,
			t_archivoConfig->PUERTO_KERNEL);

	cliente = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(cliente, (void*) &direccionKernel, sizeof(direccionKernel))
			!= 0) {
		perror("No se pudo conectar");
		return 1;
	}
	Serializar(CPU, 4, &noInteresa, cliente);

	while (1) {
		paquete* paqueteRecibido = Deserializar(cliente);
		if (paqueteRecibido->header < 0) {
			perror("Kernel se desconectó");
			return 1;
		}
		pthread_mutex_lock(&mutexProcesar);
		procesar(paqueteRecibido->package, paqueteRecibido->header,
				tamanoPaquete);
		pthread_mutex_unlock(&mutexProcesar);
	}

	free(buffer);
}
void procesar(char * paquete, int32_t id, int32_t tamanoPaquete) {
	switch (id) {
	case ARCHIVO: {
		printf("%s", paquete);
		break;
	}
	case FILESYSTEM: {
		printf("Se conecto FS");
		break;
	}
	case KERNEL: {
		printf("Se conecto Kernel\n");
		break;
	}
	case CPU: {
		printf("Se conecto CPU");
		break;
	}
	case CONSOLA: {
		printf("Se conecto Consola");
		break;
	}
	case MEMORIA: {
		memcpy(&tamanoPag, (paquete), sizeof(int));
		printf("Se conecto Memoria\n");
		sem_post(&semProcesar);
		break;
	}
	case VARIABLELEER: {
		printf("nO SERVIS PARA NADA\n");
		sem_post(&semProcesar);
		break;
	}
	case PCB: {
		programControlBlock *unPcb = deserializarPCB(paquete);
		printf("pcb id: %d\n", unPcb->programId);
		while (unPcb->exitCode != 0) {
			posicionMemoria* datos_para_memoria = malloc(
					sizeof(posicionMemoria));
			crearEstructuraParaMemoria(unPcb, tamanoPag, datos_para_memoria);
			char* sentencia = leerSentencia(datos_para_memoria->pag,
					datos_para_memoria->off, datos_para_memoria->size);
			char* barra_cero = "\0";
			memcpy(sentencia + (datos_para_memoria->size - 1), barra_cero, 1);
			analizadorLinea(depurarSentencia(sentencia), &primitivas, NULL);
			unPcb->programCounter++;
			if (unPcb->programCounter == 4)
				unPcb->exitCode = 0;

		}
		break;
	}
	}

}

char* depurarSentencia(char* sentencia) {

	int i = strlen(sentencia);
	while (string_ends_with(sentencia, "\n")) {
		i--;
		sentencia = string_substring_until(sentencia, i);
	}
	return sentencia;

}
t_puntero dummy_definirVariable(t_nombre_variable nombreVariable) {
	//REVISAAAAAAR
	printf("Entre a definir variable %c\n", nombreVariable);
	posicionMemoria *direccionVariable = malloc(sizeof(posicionMemoria));
	variable *variable = malloc(sizeof(variable));
	indiceDeStack *indiceStack = malloc(sizeof(indiceDeStack));
	indiceStack = (indiceDeStack*) (list_get(pcb->indiceStack,
			pcb->tamanoIndiceStack - 1));

	if (pcb->tamanoIndiceStack == 1 && indiceStack->tamanoVars == 0) {

		armarDireccionPrimeraPagina(direccionVariable);
		variable->etiqueta = nombreVariable;
		variable->direccion = direccionVariable;
		list_add(indiceStack->vars, variable);
		indiceStack->pos = 0;
		indiceStack->tamanoVars++;
	} else {
		armarProximaDireccion(direccionVariable);
		variable->etiqueta = nombreVariable;
		variable->direccion = direccionVariable;
		list_add(indiceStack->vars, variable);
		indiceStack->tamanoVars++;
	}

	char* escribirUMC = malloc(16);
	int valor;
	int direccionRetorno = convertirDireccionAPuntero(direccionVariable);

	enviarDirecParaEscribirMemoria(escribirUMC, direccionVariable, valor);
	free(escribirUMC);
	printf("Devuelvo direccion: %d\n", direccionRetorno);

	return (direccionRetorno);

}

t_puntero dummy_obtenerPosicionVariable(t_nombre_variable variable) {
	printf("Obtener posicion de %c\n", variable);
	return 0x10;
}

void dummy_finalizar(void) {
	printf("Finalizar\n");
}

bool terminoElPrograma(void) {
	return false;
}

t_valor_variable dummy_dereferenciar(t_puntero puntero) {
	printf("Dereferenciar %d y su valor es: %d\n", puntero, 20);
	return 20;
}

void dummy_asignar(t_puntero puntero, t_valor_variable variable) {
	printf("Asignando en %d el valor %d\n", puntero, variable);
}

void armarDireccionPrimeraPagina(posicionMemoria *direccionReal) {
	posicionMemoria *direccion = malloc(sizeof(posicionMemoria));
	direccion->off = 0;
	direccion->size = 4;
	direccion->pag = primeraPagina();
	memcpy(direccionReal, direccion, sizeof(posicionMemoria));
	free(direccion);

	return;
}

int primeraPagina() {
	return pcb->cantidadDePaginas;
}

void armarProximaDireccion(posicionMemoria* direccionReal) {
	int ultimaPosicionStack = pcb->tamanoIndiceStack - 1;
	int posicionUltimaVariable = ((indiceDeStack*) (list_get(pcb->indiceStack,
			ultimaPosicionStack)))->tamanoVars - 1;
	proximaDireccion(ultimaPosicionStack, posicionUltimaVariable,
			direccionReal);
	return;
}

void proximaDireccion(int posStack, int posUltVar,
		posicionMemoria* direccionReal) {
	posicionMemoria *direccion = malloc(sizeof(posicionMemoria));
	int offset = ((variable*) (list_get(
			((indiceDeStack*) (list_get(pcb->indiceStack, posStack)))->vars,
			posUltVar)))->direccion->off + 4;
	if (offset >= tamanoPag) {
		direccion->pag = ((variable*) (list_get(
				((indiceDeStack*) (list_get(pcb->indiceStack, posStack)))->vars,
				posUltVar)))->direccion->pag + 1;
		direccion->off = 0;
		direccion->size = 4;
		memcpy(direccionReal, direccion, sizeof(posicionMemoria));
		free(direccion);
	} else {
		direccion->pag = ((variable*) (list_get(
				((indiceDeStack*) (list_get(pcb->indiceStack, posStack)))->vars,
				posUltVar)))->direccion->pag;
		direccion->off = offset;
		direccion->size = 4;
		memcpy(direccionReal, direccion, sizeof(posicionMemoria));
		free(direccion);
	}

	return;
}

void enviarDirecParaEscribirMemoria(char* variableAEnviar,
		posicionMemoria* direccion, int valor) {

	memcpy(variableAEnviar, &direccion->pag, 4);
	memcpy(variableAEnviar + 4, &direccion->off, 4);
	memcpy(variableAEnviar + 8, &direccion->size, 4);
	memcpy(variableAEnviar + 12, &valor, 4);
	printf("Quiero escribir en la direccion: %d %d %d %d\n",
			((int*) (variableAEnviar))[0], ((int*) (variableAEnviar))[1],
			((int*) (variableAEnviar))[2], ((int*) (variableAEnviar))[3]);
	Serializar(VARIABLEESCRIBIR, 16, variableAEnviar, clienteMEM);
	//paquete * paquetin;
	//paquetin = Deserializar(clienteMEM);
	//liberar_paquete(paquetin);

}

void enviarDirecParaLeerMemoria(char* variableALeer, posicionMemoria* direccion) {

	memcpy(variableALeer, &direccion->pag, 4);
	memcpy(variableALeer + 4, &direccion->off, 4);
	memcpy(variableALeer + 8, &direccion->size, 4);
	printf("Quiero leer en la direccion: %d %d %d\n",
			((int*) (variableALeer))[0], ((int*) (variableALeer))[1],
			((int*) (variableALeer))[2]);
	Serializar(VARIABLELEER, 12, variableALeer, clienteMEM);

}

int convertirDireccionAPuntero(posicionMemoria* direccion) {

	int direccion_real, pagina, offset;
	pagina = (direccion->pag) * tamanoPag;
	offset = direccion->off;
	direccion_real = pagina + offset;
	return direccion_real;
}

void crearEstructuraParaMemoria(programControlBlock* pcb, int tamPag,
		posicionMemoria* informacion) {

	posicionMemoria* info = malloc(sizeof(posicionMemoria));
	info->pag = ceil(
			(double) pcb->indiceCodigo[(pcb->programCounter) * 2]
					/ (double) tamPag);
	printf("Voy a leer la pagina: %d\n", info->pag);
	info->off = (pcb->indiceCodigo[((pcb->programCounter) * 2)] % tamPag);
	printf("Voy a leer con offswet: %d\n", info->off);
	info->size = pcb->indiceCodigo[((pcb->programCounter) * 2) + 1];
	printf("Voy a leer ltamano: %d\n", info->size);
	memcpy(informacion, info, 12);
	free(info);
	return;
}

char* leerSentencia(int pagina, int offset, int tamanio) {
	if ((tamanio + offset) <= 20) {
		char * lecturaMemoria = malloc(12);
		posicionMemoria *datos_para_memoria = malloc(sizeof(posicionMemoria));
		datos_para_memoria->off = offset;
		datos_para_memoria->pag = pagina;
		datos_para_memoria->size = tamanio;
		enviarDirecParaLeerMemoria(lecturaMemoria, datos_para_memoria);

		paquete* instruccion2;

		instruccion2 = Deserializar(clienteMEM);
		sem_post(&semProcesar);

		printf("Codigo de operacion recibido: %d\n", instruccion2->header);

		char* sentencia2 = malloc(datos_para_memoria->size);
		memcpy(sentencia2, instruccion2->package, datos_para_memoria->size);
		free(lecturaMemoria);
		free(datos_para_memoria);
		return sentencia2;
	} else {
		int tamano1 = tamanoPag - offset;
		int tamano2 = tamanio - tamano1;
		char* lectura1 = leerSentencia(pagina, offset, tamano1);
		if (lectura1 == NULL)
			return NULL;
		char* lectura2 = leerSentencia(pagina + 1, 0, tamano2);
		if (lectura2 == NULL)
			return NULL;

		char* nuevo = malloc((20 - offset) + tamanio - (20 - offset));
		memcpy(nuevo, lectura1, (20 - offset));
		memcpy(nuevo + (20 - offset), lectura2, tamanio - (20 - offset));
		free(lectura1);
		free(lectura2);
		return nuevo;
	}
	char * lecturaMemoria = malloc(12);
	return lecturaMemoria;
}

char* procesarSentencia() {

}
