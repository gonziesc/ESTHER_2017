typedef struct{
	int PUERTO_PROG;
		int PUERTO_CPU;
		int IP_MEMORIA;
		int PUERTO_MEMORIA;
		int IP_FS;
		int PUERTO_FS;
		int QUANTUM;
		int QUANTUM_SLEEP;
		char* ALGORITMO;
		int GRADO_MULTIPROG;
		char SEM_IDS[100]; //ojo memoria
		int SEM_INIT[100];
		char SHARED_VARS[100];
		int STACK_SIZE;
}archivoConfigKernel;
