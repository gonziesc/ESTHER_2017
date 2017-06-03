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
int32_t tamanoPag = 200;
pthread_t hiloKernel;
pthread_t hiloMemoria;
int noInteresa;
AnSISOP_funciones primitivas = { .AnSISOP_definirVariable =
		dummy_definirVariable, .AnSISOP_obtenerPosicionVariable =
		dummy_obtenerPosicionVariable, .AnSISOP_dereferenciar =
		dummy_dereferenciar, .AnSISOP_asignar = dummy_asignar,
		.AnSISOP_finalizar = dummy_finalizar,

};

int32_t main(int argc, char**argv) {
	Configuracion(argv[1]);
	pthread_create(&hiloKernel, NULL, ConectarConKernel,NULL);
	//pthread_create(&hiloMemoria, NULL, conectarConMemoria,NULL);


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
	//pthread_join(hiloMemoria, NULL);
	return EXIT_SUCCESS;
}
void Configuracion(char* dir) {
	t_archivoConfig = malloc(sizeof(archivoConfigCPU));
	configuracionCpu(t_archivoConfig, config, dir);
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
	//paquete* paqueteRecibido = Deserializar(clienteMEM);
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
				tamanoPaquete);
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
		printf("Se conecto Kernel");
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
		printf("Se conecto memoria");
		break;
	}
	case PCB: {
		programControlBlock *unPcb = deserializarPCB(paquete);
		printf("pcb id: %d", unPcb->programId);
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

	printf("Entre a definir variable %c\n", nombreVariable);
	posicionMemoria *direccionVariable= malloc(sizeof(posicionMemoria));
	variable *variable= malloc(sizeof(variable));
	indiceDeStack *indiceStack = malloc(sizeof(indiceDeStack));
	indiceStack= (indiceDeStack*)(list_get(pcb->indiceStack, pcb->tamanoIndiceStack -1));

	if(pcb->tamanoIndiceStack == 1 && indiceStack->tamanoVars == 0 ){

		armarDireccionPrimeraPagina(direccionVariable);
		variable->etiqueta=nombreVariable;
		variable->direccion=direccionVariable;
		list_add(indiceStack->vars, variable);
		indiceStack->pos=0;
		indiceStack->tamanoVars++;
	}
	else {
		armarProximaDireccion(direccionVariable);
		variable->etiqueta=nombreVariable;
		variable->direccion=direccionVariable;
		list_add(indiceStack->vars, variable);
		indiceStack->tamanoVars++;
	}

	char* escribirUMC= malloc(16);
	int valor;
	int direccionRetorno = convertirDireccionAPuntero(direccionVariable);

	enviarDirecParaEscribirUMC(escribirUMC, direccionVariable, valor);
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

void armarDireccionPrimeraPagina(posicionMemoria *direccionReal){
	posicionMemoria  *direccion = malloc(sizeof(posicionMemoria));
	direccion->off=0;
	direccion->size=4;
	direccion->pag=primeraPagina();
	memcpy(direccionReal, direccion , sizeof(posicionMemoria ));
	free(direccion);

	return;
}

int primeraPagina(){
	return pcb->cantidadDePaginas;
}

void armarProximaDireccion(posicionMemoria* direccionReal){
	int ultimaPosicionStack = pcb->tamanoIndiceStack-1;
	int posicionUltimaVariable = ((indiceDeStack*)(list_get(pcb->indiceStack, ultimaPosicionStack)))->tamanoVars-1;
	proximaDireccion(ultimaPosicionStack, posicionUltimaVariable, direccionReal);
	return;
}

void proximaDireccion(int posStack, int posUltVar, posicionMemoria* direccionReal){
	posicionMemoria *direccion = malloc(sizeof(posicionMemoria));
	int offset = ((variable*)(list_get(((indiceDeStack*)(list_get(pcb->indiceStack, posStack)))->vars, posUltVar)))->direccion->off+ 4;
		if(offset>=tamanoPag){
			direccion->pag= ((variable*)(list_get(((indiceDeStack*)(list_get(pcb->indiceStack, posStack)))->vars, posUltVar)))->direccion->pag+ 1;
			direccion->off= 0;
			direccion->size=4;
			memcpy(direccionReal, direccion , sizeof(posicionMemoria));
			free(direccion);
		}else{
			direccion->pag= ((variable*)(list_get(((indiceDeStack*)(list_get(pcb->indiceStack, posStack)))->vars, posUltVar)))->direccion->pag;
			direccion->off= offset;
			direccion->size=4;
			memcpy(direccionReal, direccion , sizeof(posicionMemoria));
			free(direccion);
		}

		return;
}

void enviarDirecParaEscribirUMC(char* variableAEnviar, posicionMemoria* direccion, int valor){

		memcpy(variableAEnviar, &direccion->pag, 4);
		memcpy(variableAEnviar+4, &direccion->off, 4);
		memcpy(variableAEnviar+8, &direccion->size , 4);
		memcpy(variableAEnviar+12, &valor , 4);
		printf("Quiero escribir en la direccion: %d %d %d %d\n",((int*)(variableAEnviar))[0],((int*)(variableAEnviar))[1],((int*)(variableAEnviar))[2],((int*)(variableAEnviar))[3]);
		Serializar(VARIABLE, 16, variableAEnviar,clienteMEM);
		//paquete * paquetin;
		//paquetin = Deserializar(clienteMEM);
		//liberar_paquete(paquetin);

}

int convertirDireccionAPuntero(posicionMemoria* direccion){

	int direccion_real,pagina,offset;
	pagina=(direccion->pag)*tamanoPag;
	offset=direccion->off;
	direccion_real=pagina+offset;
	return direccion_real;
}
