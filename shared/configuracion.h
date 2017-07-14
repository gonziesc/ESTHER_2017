#ifndef CONFIGURACION_H_
#define CONFIGURACION_H_

#include <commons/config.h>
#include<stdio.h>
#include<stdlib.h>
#include <string.h>

typedef enum {
	OK = 0,
	ARCHIVO = 1,
	PCB = 2,
	MEMORIA = 3,
	FILESYSTEM = 4,
	KERNEL = 5,
	CPU = 6,
	CONSOLA = 7,
	CODIGO = 8,
	TAMANO = 9,
	PAGINA = 10,
	PID = 11,
	LEERSENTENCIA = 12,
	ESCRIBIRVARIABLE = 13,
	MATARPIDPORCONSOLA = 14,
	PROGRAMATERMINADO = 15,
	PROGRAMAABORTADO = 16,
	DATOSPLANIFICACION = 17,
	PAGINAENVIADA = 18,
	ENTRAPROCESO = 19,
	PUNTEROPAGINAHEAP = 20,
	DEREFERENCIAR = 21,
	PIDOPAGINAHEAP = 22,
	ESCRITURAPAGINA = 23,
	FINALIZOPROGRAMA = 24,
	IMPRESIONPORCONSOLA = 25,
	DESCONECTARCONSOLA = 26,
	DATOSKERNELCPU = 27,
	FINDEQUATUM = 28,
	INICIALIZARPROCESO = 30,
	NOENTROPROCESO = 29,
	VALORVARIABLECOMPARTIDA = 31,
	ASIGNOVALORVARIABLECOMPARTIDA = 32,
	IMPRIMIRPROCESO = 33,
	PROCESOPIDEHEAP = 34,
	PROCESOLIBERAHEAP = 35,
	PROCESOFINALIZAO = 36,
	PROCESOWAIT = 37,
	PROCESOSIGNAL = 38,
	PROCESOABRIRARCHIVO = 39,
	PROCESOBORRARARCHIVO = 40,
	PROCESOCERRARARCHIVO = 41,
	PROCESOMOVERCURSORARCHIVO = 42,
	PROCESOLEERARCHIVO = 43,
	PROCESOESCRIBIRARCHIVO = 44,
	PROCESOBLOQUEADO = 45,
	ABORTOPORMASRESERVERAQUEPAGINA = 46,
	ABORTOARCHIVONOEXISTE = 47,
	ABORTOLEERSINPERMISOS = 48,
	ABORTOESCRIBIRSINPERMISOS = 49,
	ABORTOEXPECIONDEMEMORIA = 50,
	PROCESODESBLOQUEADO = 51,
	ABORTOSTACKOVERFLOW = 52,
	SEBLOQUEOELPROCESO = 53,
	PROCESOTERMINALIBERAHEAP = 54,
	MEMORIARESERVOHEAP = 55,
	METADATALEIDA = 56,
	VALIDARARCHIVO = 57,
	CREARARCHIVO = 58,
	BORRARARCHIVO = 59,
	OBTENERDATOS = 60,
	GUARDARDATOS = 61,
	ABRIRARCHIVO = 62,
	CERRARARCHIVO = 63,
	ESCRIBIRARCHIVO = 64,
	LEERARCHIVO = 65,
	MOVERCURSOR = 66,
	BORRARARCHIVOFS = 67,
	ABORTOPORCONSOLA = 68,
	PORLASDUDAS = 69,
	LIBERARPAGINAS = 70,
	ABORTONOHAYLUGARFS = 71,
	ABORTOGRADOMULTIPROG = 72

} codigosSerializador;

typedef enum {
	codeFinalizoCorrectamente = 0,
	codeFaltanRecursos = -1,
	codeArchivoNoexiste = -2,
	codeLeerSinPermisos = -3,
	codeEscribirSinPermisos = -4,
	codeExcepcionMemoria = -5,
	codeDesconexionConsola = -6,
	codeFinalizarPrograma = -7,
	codeMasMemoriaQuePaginas = -8,
	codeNoSePuedeAsignarMasPaginas = -9,
	codeStackOverflow = -10,
	codeDesconexionCpu = -11,
	codeNoHayMasLugarFS = -12,
	codeDesconocido = -20

}exitCode;

typedef struct{
	int32_t PUERTO;
	int32_t MARCOS;
	int32_t MARCOS_SIZE;
	int32_t ENTRADAS_CACHE;
	int32_t CACHE_X_PROC;
	char* REEMPLAZO_CACHE;
	int32_t RETARDO_MEMORIA;
}archivoConfigMemoria;

typedef struct{
	int32_t PUERTO_PROG;
	int32_t PUERTO_CPU;
		char* IP_MEMORIA;
		int32_t PUERTO_MEMORIA;
		char* IP_FS;
		int32_t PUERTO_FS;
		int32_t QUANTUM;
		int32_t QUANTUM_SLEEP;
		char* ALGORITMO;
		int32_t GRADO_MULTIPROG;
		char** SEM_IDS; //ojo memoria
		char** SEM_INIT;
		char** SHARED_VARS;
		char** SHARED_VARS_INIT;
		int32_t STACK_SIZE;
}archivoConfigKernel;

typedef struct{
	char *IP_KERNEL;
	int32_t PUERTO_KERNEL;
	char *IP_MEMORIA;
	int32_t PUERTO_MEMORIA;
}archivoConfigCPU;


typedef struct{
	char *IP_KERNEL;
	int32_t PUERTO_KERNEL;
}archivoConfigConsola;


typedef struct{
	char *PUERTO_MONTAJE;
	int32_t PUERTO_KERNEL;
	char *MAGIC_NUMBER;
	int32_t TAMANIO_BLOQUES;
	int32_t CANTIDAD_BLOQUES;
}archivoConfigFS;

void configuracionMemoria(archivoConfigMemoria *, t_config* , char *);
void configuracionKernel(archivoConfigKernel *, t_config* , char *);
void configuracionConsola(archivoConfigConsola *, t_config* , char *);
void configuracionFS(archivoConfigFS *, t_config* , char *, char* );
void configuracionCpu(archivoConfigCPU *, t_config* , char *);

#endif
