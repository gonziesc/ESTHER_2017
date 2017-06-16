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
programControlBlock *unPcb;
int32_t tamanoPag;
pthread_t hiloKernel;
pthread_t hiloMemoria;
pthread_t hiloProcesarScript;
pthread_mutex_t mutexProcesar;
sem_t semProcesar;
sem_t semInstruccion;
sem_t semSentenciaCompleta;
sem_t semHayScript;
sem_t okDeMemoria;
sem_t semDereferenciar;
int noInteresa;
int valorDerenferenciado;
char * instruccionLeida;
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
	pthread_create(&hiloProcesarScript, NULL, procesarScript, NULL);

	pthread_join(hiloKernel, NULL);
	pthread_join(hiloMemoria, NULL);
	pthread_join(hiloProcesarScript, NULL);
	return EXIT_SUCCESS;
}
void Configuracion(char* dir) {
	t_archivoConfig = malloc(sizeof(archivoConfigCPU));
	configuracionCpu(t_archivoConfig, config, dir);
	sem_init(&semProcesar, 0, 1);
	sem_init(&semSentenciaCompleta, 0, 0);
	sem_init(&semInstruccion, 0, 0);
	sem_init(&semHayScript, 0, 0);
	sem_init(&okDeMemoria, 0, 0);
	sem_init(&semDereferenciar, 0, 0);
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
		//sem_wait(&semProcesar);
		paquete* paqueteRecibido = Deserializar(clienteMEM);
		if (paqueteRecibido->header < 0) {
			perror("Memoria se desconectó");
			return 1;
		}
		procesar(paqueteRecibido->package, paqueteRecibido->header, paqueteRecibido->size);
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
		procesar(paqueteRecibido->package, paqueteRecibido->header,
				paqueteRecibido->size);
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
		break;
	}
	case VARIABLELEER: {
		instruccionLeida = malloc(tamanoPaquete);
		memcpy(instruccionLeida, paquete, tamanoPaquete);
		sem_post(&semInstruccion);
		break;
	}
	case VARIABLEESCRIBIR: {
			sem_post(&okDeMemoria);
			break;
		}
	case DEREFERENCIAR: {
		memcpy(&valorDerenferenciado, paquete, 4);
		sem_post(&semDereferenciar);
		break;
	}
	case PCB: {
		unPcb = deserializarPCB(paquete);
		printf("unPcb id: %d\n", unPcb->programId);
		sem_post(&semHayScript);
		break;
	}
	}

}

void procesarScript() {
	while (1) {
		sem_wait(&semHayScript);
		while (unPcb->exitCode != 0) {
			posicionMemoria* datos_para_memoria = malloc(
					sizeof(posicionMemoria));
			crearEstructuraParaMemoria(unPcb, tamanoPag, datos_para_memoria);
			char* sentencia = leerSentencia(datos_para_memoria->pag,
					datos_para_memoria->off, datos_para_memoria->size, 0);
			char* barra_cero = "\0";
			memcpy(sentencia + (datos_para_memoria->size - 1), barra_cero, 1);
			sem_wait(&semSentenciaCompleta);
			//printf("sentencia leida:%s \n", sentencia);
			analizadorLinea(depurarSentencia(sentencia), &primitivas, NULL);
			unPcb->programCounter++;
			if (unPcb->programCounter == 4)
				unPcb->exitCode = 0;

		}
		Serializar(PROGRAMATERMINADO, 4, &noInteresa, cliente);
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
	variable *unaVariable = malloc(sizeof(variable));
	indiceDeStack *indiceStack = malloc(sizeof(indiceDeStack));
	indiceStack = (indiceDeStack*) (list_get(unPcb->indiceStack,
			unPcb->tamanoIndiceStack - 1));

	if (unPcb->tamanoIndiceStack == 1 && indiceStack->tamanoVars == 0) {

		armarDireccionPrimeraPagina(direccionVariable);
		unaVariable->etiqueta = nombreVariable;
		unaVariable->direccion = direccionVariable;
		//OJO DIRECCION VARIABLE NO TIENE NADA...
		list_add(indiceStack->vars, unaVariable);
		indiceStack->pos = 0;
		indiceStack->tamanoVars++;
	} else {
		armarProximaDireccion(direccionVariable);
		unaVariable->etiqueta = nombreVariable;
		unaVariable->direccion = direccionVariable;
		list_add(indiceStack->vars, unaVariable);
		indiceStack->tamanoVars++;
	}
	int valor = 0;
	int direccionRetorno = convertirDireccionAPuntero(direccionVariable);

	enviarDirecParaEscribirMemoria(direccionVariable, valor);
	printf("Devuelvo direccion: %d\n", direccionRetorno);

	return (direccionRetorno);

}

t_puntero dummy_obtenerPosicionVariable(t_nombre_variable nombreVariable) {
    printf("Obtener posicion de %c\n", nombreVariable);
    int posicionStack= unPcb->tamanoIndiceStack-1;
    int direccionRetorno;
    variable *variableNueva;
    int posMax= (((indiceDeStack*)(list_get(unPcb->indiceStack, posicionStack)))->tamanoVars)-1;
    while(posMax>=0){
        variableNueva=((variable*)(list_get(((indiceDeStack*)(list_get(unPcb->indiceStack, posicionStack)))->vars, posMax)));
        printf("Variable: %c\n", variableNueva->etiqueta);
        if(variableNueva->etiqueta==nombreVariable){
            direccionRetorno = convertirDireccionAPuntero(((variable*)(list_get(((indiceDeStack*)(list_get(unPcb->indiceStack, posicionStack)))->vars, posMax)))->direccion);
            printf("Obtengo valor de %c: %d %d %d\n", variableNueva->etiqueta, variableNueva->direccion->pag, variableNueva->direccion->off, variableNueva->direccion->size);
            return(direccionRetorno);
        }
        posMax--;
    }
}

void dummy_finalizar(void) {
    printf("Finalizar\n");
}

bool terminoElPrograma(void) {
    return false;
}

t_valor_variable dummy_dereferenciar(t_puntero puntero) {
    posicionMemoria *direccion= malloc(sizeof(posicionMemoria));
    convertirPunteroADireccion(puntero, direccion);
    enviarDirecParaLeerMemoria(direccion, DEREFERENCIAR);
    sem_wait(&semDereferenciar);
    free(direccion);
    printf("Dereferenciar %d y su valor es: %d\n", puntero, valorDerenferenciado);
    return valorDerenferenciado;
}

void dummy_asignar(t_puntero punteroAVariable, t_valor_variable valor) {
    printf("Asignando en %d el valor %d\n", punteroAVariable, valor);
    posicionMemoria *direccion= malloc(sizeof(posicionMemoria));
    convertirPunteroADireccion(punteroAVariable, direccion);
    //ARREGLAR INAKI
    enviarDirecParaEscribirMemoria(direccion, valor);
    //falta el semaforo de ok
    free(direccion);
    return;
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
	return unPcb->cantidadDePaginas;
}

void armarProximaDireccion(posicionMemoria* direccionReal) {
	int ultimaPosicionStack = unPcb->tamanoIndiceStack - 1;
	int posicionUltimaVariable = ((indiceDeStack*) (list_get(unPcb->indiceStack,
			ultimaPosicionStack)))->tamanoVars - 1;
	proximaDireccion(ultimaPosicionStack, posicionUltimaVariable,
			direccionReal);
	return;
}

void proximaDireccion(int posStack, int posUltVar,
		posicionMemoria* direccionReal) {
	posicionMemoria *direccion = malloc(sizeof(posicionMemoria));
	int offset = ((variable*) (list_get(
			((indiceDeStack*) (list_get(unPcb->indiceStack, posStack)))->vars,
			posUltVar)))->direccion->off + 4;
	if (offset >= tamanoPag) {
		direccion->pag =
				((variable*) (list_get(
						((indiceDeStack*) (list_get(unPcb->indiceStack,
								posStack)))->vars, posUltVar)))->direccion->pag
						+ 1;
		direccion->off = 0;
		direccion->size = 4;
		memcpy(direccionReal, direccion, sizeof(posicionMemoria));
		free(direccion);
	} else {
		direccion->pag =
				((variable*) (list_get(
						((indiceDeStack*) (list_get(unPcb->indiceStack,
								posStack)))->vars, posUltVar)))->direccion->pag;
		direccion->off = offset;
		direccion->size = 4;
		memcpy(direccionReal, direccion, sizeof(posicionMemoria));
		free(direccion);
	}

	return;
}

void enviarDirecParaEscribirMemoria(posicionMemoria* direccion, int valor) {
	char* variableAEnviar = malloc(16);
	memcpy(variableAEnviar, &direccion->pag, 4);
	memcpy(variableAEnviar + 4, &direccion->off, 4);
	memcpy(variableAEnviar + 8, &direccion->size, 4);
	memcpy(variableAEnviar + 12, &valor, 4);
	printf("Quiero escribir en la direccion: %d %d %d %d\n",
			((int*) (variableAEnviar))[0], ((int*) (variableAEnviar))[1],
			((int*) (variableAEnviar))[2], ((int*) (variableAEnviar))[3]);
	Serializar(VARIABLEESCRIBIR, 16, variableAEnviar, clienteMEM);
	free(variableAEnviar);
	//paquete * paquetin;
	//paquetin = Deserializar(clienteMEM);
	//liberarPaquete(paquetin);

}

void enviarDirecParaLeerMemoria(posicionMemoria* direccion, int header) {
	char * variableALeer = malloc(12);
	memcpy(variableALeer, &direccion->pag, 4);
	memcpy(variableALeer + 4, &direccion->off, 4);
	memcpy(variableALeer + 8, &direccion->size, 4);
	printf("Quiero leer en la direccion: %d %d %d\n",
			((int*) (variableALeer))[0], ((int*) (variableALeer))[1],
			((int*) (variableALeer))[2]);
	Serializar(header, 12, variableALeer, clienteMEM);
	free(variableALeer);

}

int convertirDireccionAPuntero(posicionMemoria* direccion) {

	int direccion_real, pagina, offset;
	pagina = (direccion->pag) * tamanoPag;
	offset = direccion->off;
	direccion_real = pagina + offset;
	return direccion_real;
}

void convertirPunteroADireccion(int puntero, posicionMemoria* direccion){
	if(tamanoPag>puntero){
			direccion->pag=0;
				direccion->off=puntero;
					direccion->size=4;
			direccion->pag = (puntero/tamanoPag);
			direccion->off = puntero%tamanoPag;
			direccion->size=4;
	}
	return;
}


void crearEstructuraParaMemoria(programControlBlock* unPcb, int tamPag,
		posicionMemoria* informacion) {

	posicionMemoria* info = malloc(sizeof(posicionMemoria));
	info->pag = ceil(
			(double) unPcb->indiceCodigo[(unPcb->programCounter) * 2]
					/ (double) tamPag);
	printf("Voy a leer la pagina: %d\n", info->pag);
	info->off = (unPcb->indiceCodigo[((unPcb->programCounter) * 2)] % tamPag);
	printf("Voy a leer con offswet: %d\n", info->off);
	info->size = unPcb->indiceCodigo[((unPcb->programCounter) * 2) + 1];
	printf("Voy a leer el tamano: %d\n", info->size);
	memcpy(informacion, info, 12);
	free(info);
	return;
}

char* leerSentencia(int pagina, int offset, int tamanio, int flag) {
	if ((tamanio + offset) <= 20) {
		posicionMemoria *datos_para_memoria = malloc(sizeof(posicionMemoria));
		datos_para_memoria->off = offset;
		datos_para_memoria->pag = pagina;
		datos_para_memoria->size = tamanio;
		enviarDirecParaLeerMemoria(datos_para_memoria, VARIABLELEER);
		sem_wait(&semInstruccion);
		char* sentencia2 = malloc(datos_para_memoria->size);
		memcpy(sentencia2, instruccionLeida, datos_para_memoria->size);
		free(datos_para_memoria);
		if (flag == 0)
			sem_post(&semSentenciaCompleta);
		return sentencia2;
	} else {
		int tamano1 = tamanoPag - offset;
		int tamano2 = tamanio - tamano1;
		char* lectura1 = leerSentencia(pagina, offset, tamano1, 1);
		if (lectura1 == NULL)
			return NULL;
		char* lectura2 = leerSentencia(pagina + 1, 0, tamano2, 1);
		if (lectura2 == NULL)
			return NULL;

		char* nuevo = malloc((20 - offset) + tamanio - (20 - offset));
		memcpy(nuevo, lectura1, (20 - offset));
		memcpy(nuevo + (20 - offset), lectura2, tamanio - (20 - offset));
		free(lectura1);
		free(lectura2);
		sem_post(&semSentenciaCompleta);
		return nuevo;
	}
	char * lecturaMemoria = malloc(12);
	return lecturaMemoria;
}

char* procesarSentencia() {

}
