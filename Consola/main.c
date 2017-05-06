#include "main.h"


archivoConfigConsola* t_archivoConfig;
t_config *config;
struct sockaddr_in direccionKernel;direccionMem;
int32_t cliente; clienteMEM; buffer; bytesRecibidos; idHiloLeerComando;idHiloConectarseConKernel;
pthread_t hiloLeerComando; hiloConectarseConKernel;

int32_t main(int argc, char**argv) {
	Configuracion(argv[1]);
	idHiloConectarseConKernel = pthread_create(&hiloConectarseConKernel, NULL, ConectarseConKernel, NULL);

	idHiloLeerComando = pthread_create(&hiloLeerComando, NULL, leerComando, NULL);
	pthread_join(hiloConectarseConKernel, NULL);
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
	Serializar(CONSOLA, 4, 0, cliente);
	//DESERIALIZAR Y PROCESAR
	return 69; // para que retorne un int (cualquiera)
}

void leerComando()
{
	while (1) {
		char *mensaje = malloc(100);
		fgets(mensaje, 100, stdin);
		pthread_t hiloUnico;
		int32_t idHiloUnico;
		idHiloUnico = pthread_create(&hiloUnico, NULL, crearNuevoProceso, (char*) mensaje);
		pthread_join(hiloUnico, NULL);
	}
}

void crearNuevoProceso(char* path)
{
	char *contenidoDelArchivo = malloc(100);
	int tamano = abrirYLeerArchivo(path, contenidoDelArchivo);
	Serializar(1, tamano, contenidoDelArchivo, cliente);
	free(contenidoDelArchivo);
}

int abrirYLeerArchivo(char* path, char* string)
{
	//ojoooooooooo
	FILE *f = fopen("/home/utnso/git/tp-2017-1c-el-grupo-numero/Consola/hola.ansisop", "rb");
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);  //same as rewind(f);
	fread(string, fsize, 1, f);
	fclose(f);
	return fsize;
}
