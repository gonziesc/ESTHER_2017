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

AnSISOP_funciones primitivas = {
		.AnSISOP_definirVariable		= dummy_definirVariable,
		.AnSISOP_obtenerPosicionVariable= dummy_obtenerPosicionVariable,
		.AnSISOP_dereferenciar			= dummy_dereferenciar,
		.AnSISOP_asignar				= dummy_asignar,
		.AnSISOP_finalizar				= dummy_finalizar,

};

int32_t main(int argc, char**argv) {
	Configuracion(argv[1]);
	char* sentencia = "a = b + 12";
	analizadorLinea(depurarSentencia(sentencia), &primitivas, NULL);
	ConectarConKernel();
	//conectarConMemoria();
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
	Serializar(6, 0, 0, clienteMEM);

	while (1) {
		int32_t bytesRecibidos = recv(clienteMEM, &header, 4, 0);
		if (bytesRecibidos <= 0) {
			perror("El chabón se desconectó o bla.");
			return 1;
		}
		Deserializar(header, 0);
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
	Serializar(CPU, 0, 0, cliente);

	while (1) {
		int32_t bytesRecibidos = recv(cliente, &header, 4, 0);
		if (bytesRecibidos <= 0) {
			perror("Kernel se desconectó");
			return 1;
		}
		char * paquete = Deserializar(header, 0, cliente, &tamanoPaquete);
		procesar(paquete,header, tamanoPaquete);
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
	case CODIGO: {

	}
	}
}

char* depurarSentencia(char* sentencia){

		int i = strlen(sentencia);
		while (string_ends_with(sentencia, "\n")) {
			i--;
			sentencia = string_substring_until(sentencia, i);
		}
		return sentencia;

}
t_puntero dummy_definirVariable(t_nombre_variable variable) {
	printf("definir la variable %c\n", variable);
	return 0x10;
}

t_puntero dummy_obtenerPosicionVariable(t_nombre_variable variable) {
	printf("Obtener posicion de %c\n", variable);
	return 0x10;
}

void dummy_finalizar(void){
	printf("Finalizar\n");
}

bool terminoElPrograma(void){
	return false;
}

t_valor_variable dummy_dereferenciar(t_puntero puntero) {
	printf("Dereferenciar %d y su valor es: %d\n", puntero, 20);
	return 20;
}

void dummy_asignar(t_puntero puntero, t_valor_variable variable) {
	printf("Asignando en %d el valor %d\n", puntero, variable);
}

