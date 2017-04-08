typedef struct{
	int PUERTO_PROG;
		int PUERTO_CPU;
		char* IP_MEMORIA;
		int PUERTO_MEMORIA;
		char* IP_FS;
		int PUERTO_FS;
		int QUANTUM;
		int QUANTUM_SLEEP;
		char* ALGORITMO;
		int GRADO_MULTIPROG;
		char** SEM_IDS; //ojo memoria
		char** SEM_INIT;
		char** SHARED_VARS;
		int STACK_SIZE;
}archivoConfigKernel;
