#include "main.h"


archivoConfigConsola* t_archivoConfig;
t_config *config;
struct sockaddr_in direccionKernel;direccionMem;
int32_t cliente; clienteMEM; buffer; bytesRecibidos; idHiloLeerComando;
pthread_t hiloLeerComando;

int32_t main(int argc, char**argv) {
	Configuracion(argv[1]);
	ConectarseConKernel();
	idHiloLeerComando = pthread_create(&hiloLeerComando, NULL, leerComando, NULL);
	pthread_join(hiloLeerComando, NULL);
	return EXIT_SUCCESS;
}
void Configuracion(char* dir) {
	t_archivoConfig = malloc(sizeof(archivoConfigConsola));
	configuracionConsola(t_archivoConfig, config, dir);
}
int32_t ConectarseConKernel() {
	llenarSocketAdrrConIp(&direccionKernel, t_archivoConfig->IP_KERNEL,
			t_archivoConfig->PUERTO_KERNEL);
	cliente = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(cliente, (void*) &direccionKernel, sizeof(direccionKernel))
			!= 0) {
		perror("No se pudo conectar");
		return 1;
	}
	send(cliente, "hola, soy consola", sizeof("hola, soy consola"), 0);
}

void leerComando()
{
	while (1) {
		char mensaje[100];
		fgets(mensaje, sizeof(mensaje), stdin);
		pthread_t hiloUnico;
		int32_t idHiloUnico;
		idHiloUnico = pthread_create(&hiloUnico, NULL, crearNuevoProceso, (void*) mensaje);
		pthread_join(hiloUnico, NULL);
	}
}

void crearNuevoProceso(void* arg)
{
	char *path = (char *) arg;
	char contenidoDelArchivo[100];
	int tamano = abrirYLeerArchivo(path, &contenidoDelArchivo);
	printf("Archivo leido");
	send(cliente, contenidoDelArchivo, tamano, 0);
	printf("Mensaje recibido y enviado a kernel\n");
	printf("Archivo: %s", contenidoDelArchivo);
}


int abrirYLeerArchivo(char* path, char* string)
{
	//ojoooooooooo
	FILE *f = fopen("/home/utnso/tp-2017-1c-el-grupo-numero/Consola/hola.ansisop", "rb");
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);  //same as rewind(f);
	fread(string, fsize, 1, f);
	fclose(f);
	return fsize;
}
