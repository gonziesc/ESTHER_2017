#include "main.h"

int32_t opcion;
char nombreArchivo[100];
archivoConfigConsola* t_archivoConfig;
t_config *config;
struct sockaddr_in direccionKernel;
direccionMem;
int32_t cliente;
procesosActualesPosicion = 0;
header;
tamanoPaquete;
buffer;
bytesRecibidos;
idHiloLeerComando;
idHiloConectarseConKernel;
pthread_t hiloLeerComando;
hiloConectarseConKernel;
ProcesosActuales procesosActuales[100];
int noInteresa;
sem_t semPidListo;
int pidActual;

int32_t main(int argc, char**argv) {
	Configuracion(argv[1]);
	idHiloConectarseConKernel = pthread_create(&hiloConectarseConKernel, NULL,
			ConectarseConKernel, noInteresa);
	idHiloLeerComando = pthread_create(&hiloLeerComando, NULL, leerComando,
	NULL);
	pthread_join(hiloConectarseConKernel, NULL);
	pthread_join(hiloLeerComando, NULL);
	return EXIT_SUCCESS;
}
void Configuracion(char* dir) {
	t_archivoConfig = malloc(sizeof(archivoConfigConsola));
	configuracionConsola(t_archivoConfig, config, dir);
	sem_init(&semPidListo, 0, 0);
}
int32_t ConectarseConKernel(int noIMporta) {
	llenarSocketAdrrConIp(&direccionKernel, t_archivoConfig->IP_KERNEL,
			t_archivoConfig->PUERTO_KERNEL);
	cliente = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(cliente, (void*) &direccionKernel, sizeof(direccionKernel))
			!= 0) {
		perror("No se pudo conectar \n");
		return 1;
	}

	Serializar(CONSOLA, 4, &noIMporta, cliente);
	while (1) {
		//sem_wait(&semProcesar);
		paquete* paqueteRecibido = Deserializar(cliente);
		if (paqueteRecibido->header < 0) {
			perror("Kernel se desconectó \n");
			return 1;
		}
		procesar(paqueteRecibido->package, paqueteRecibido->header,
				paqueteRecibido->size);
	}

}

void procesar(char * paquete, int32_t id, int32_t tamanoPaquete) {
	switch (id) {
	case IMPRESIONPORCONSOLA: {
		char * impresion = malloc(tamanoPaquete - 4 + 1);
		memcpy(impresion, paquete, tamanoPaquete - 4);
		int pid;
		memcpy(&pid, paquete + tamanoPaquete -4, sizeof(int));
		imprimioProceso(pid);
		impresion[tamanoPaquete] = '\0';
		printf("el pid %d imprimio: %s \n", pid, impresion);
		break;
	}
	case FINALIZOPROGRAMA: {
		int pid;
		char* fechaFIn = temporal_get_string_time();
		memcpy(&pid, paquete, sizeof(int));
		ProcesosActuales procesoTerminado = buscarProceso(pid);
		printf("el pid %d comenzo a las %s \n", pid,
				procesoTerminado.horaInicio);
		printf("el pid %d finalizo a las %s \n", pid, fechaFIn);
		printf("el pid %d imprimio la cantidad de : %d \n", pid,
				procesoTerminado.cantidadDeImpresiones);
		break;
	}
	case PID: {
		memcpy(&pidActual, paquete, sizeof(int));
		sem_post(&semPidListo);
	}
	}
}

void leerComando() {
	while (1) {
		printf("Ingrese comando\n");
		printf("1: iniciar programa mandando path absoluto \n");
		printf("2: finalizar programa ingresando pid\n");
		printf("3: desconectar consolsa\n");
		printf("4: limpiar mensajes\n");
		scanf("%d", &opcion);
		switch (opcion) {
		case 1: {
			pthread_t hiloPrograma;
			int32_t idHiloId;
			idHiloId = pthread_create(&hiloPrograma, NULL, crearNuevoProceso,
					procesosActualesPosicion);
			procesosActuales[procesosActualesPosicion].identificadorHilo =
					idHiloId;
			procesosActualesPosicion++;
			pthread_join(hiloPrograma, NULL);
			break;
		}
		case 2: {
			int pidAMatar;
			printf("Ingrese pid a finalizar\n");
			scanf("%d", &pidAMatar);
			Serializar(MATARPIDPORCONSOLA, 4, &pidAMatar, cliente);
			char* fechaFIn = temporal_get_string_time();
			ProcesosActuales procesoTerminado = buscarProceso(pidAMatar);
			printf("el pid %d comenzo a las %s \n", pidAMatar,
					procesoTerminado.horaInicio);
			printf("el pid %d finalizo a las %s \n", pidAMatar, fechaFIn);
			printf("el pid %d imprimio la cantidad de : %d \n", pidAMatar,
					procesoTerminado.cantidadDeImpresiones);
			pthread_cancel(procesosActuales[pidAMatar].identificadorHilo);
			break;
		}
		case 3: {
			matarTodosLosProcesos();
			Serializar(DESCONECTARCONSOLA, 4, &noInteresa, cliente);
			close(cliente);
			pthread_cancel(idHiloConectarseConKernel);
			break;
		}
		case 4: {
			printf("\e[1;1H\e[2J");
			break;
		}
		}
	}
}

void crearNuevoProceso(int procesosActualesPosicion) {
	printf("Ingrese la ruta del archivo\n");
	scanf("%s", &nombreArchivo);
	char *contenidoDelArchivo = malloc(1000);
	int tamano = abrirYLeerArchivo(nombreArchivo, contenidoDelArchivo);
	Serializar(ARCHIVO, tamano, contenidoDelArchivo, cliente);
	sem_wait(&semPidListo);
	procesosActuales[procesosActualesPosicion].PID = pidActual;
	procesosActuales[procesosActualesPosicion].horaInicio =
			temporal_get_string_time();
	printf("process id: %d\n", pidActual);

}

ProcesosActuales buscarProceso(int pid) {
	int i;
	for (i = 0; i <= 100; i++) {
		if (procesosActuales[i].PID == pid) {
			return procesosActuales[i];
		}
	}
}

void imprimioProceso(int pid) {
	int i;
	for (i = 0; i <= 100; i++) {
		if (procesosActuales[i].PID == pid) {
			procesosActuales[i].cantidadDeImpresiones++;
		}
	}
}

void matarTodosLosProcesos() {
	int i;
	for (i = 0; i <= 100; i++) {
		if (procesosActuales[i].PID != 0) {
			char* fechaFIn = temporal_get_string_time();
			printf("el pid %d comenzo a las %s \n", procesosActuales[i].PID,
					procesosActuales[i].horaInicio);
			printf("el pid %d finalizo a las %s \n", procesosActuales[i].PID,
					fechaFIn);
			printf("el pid %d imprimio la cantidad de : %d \n",
					procesosActuales[i].PID,
					procesosActuales[i].cantidadDeImpresiones);
		}
	}
}

int abrirYLeerArchivo(char path[], char* string) {

	FILE *f = fopen(path, "rb");
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);  //same as rewind(f);
	fread(string, fsize, 1, f);
	fclose(f);
	return fsize;
}
