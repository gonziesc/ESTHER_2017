#include "configuracion.h"






void configuracionMemoria(archivoConfigMemoria *unArchivo, t_config* config, char *dir){

		config = config_create(dir);

		unArchivo->PUERTO = config_get_int_value(config, "PUERTO");
		printf("PUERTO: %d\n", unArchivo->PUERTO);

		unArchivo->MARCOS = config_get_int_value(config, "MARCOS");
		printf("MARCOS: %d\n", unArchivo->MARCOS);

		unArchivo->MARCOS_SIZE = config_get_int_value(config, "MARCOS_SIZE");
		printf("MARCOS_SIZE: %d\n", unArchivo->MARCOS_SIZE);

		unArchivo->ENTRADAS_CACHE = config_get_int_value(config, "ENTRADAS_CACHE");
		printf("ENTRADAS_CACHE: %d\n", unArchivo->ENTRADAS_CACHE);

		unArchivo->CACHE_X_PROC = config_get_int_value(config, "CACHE_X_PROC");
		printf("CACHE_X_PROC: %d\n", unArchivo->CACHE_X_PROC);

		unArchivo->REEMPLAZO_CACHE = config_get_string_value(config, "REEMPLAZO_CACHE");
		printf("REEMPLAZO_CACHE: %s\n", unArchivo->REEMPLAZO_CACHE);

		unArchivo->RETARDO_MEMORIA = config_get_int_value(config, "RETARDO_MEMORIA");
		printf("RETARDO_MEMORIA: %d\n", unArchivo->RETARDO_MEMORIA);

}

void configuracionKernel(archivoConfigKernel *unArchivo, t_config* config, char *dir){
	int32_t i = 0;
	config = config_create(dir);
	unArchivo->PUERTO_CPU = config_get_int_value(config, "PUERTO_CPU");
	printf("PUERTO_CPU: %d\n", unArchivo->PUERTO_CPU);
	unArchivo->PUERTO_PROG = config_get_int_value(config, "PUERTO_PROG");
	printf("PUERTO_PROG: %d\n", unArchivo->PUERTO_PROG);
	unArchivo->IP_MEMORIA  = config_get_string_value(config, "IP_MEMORIA");
	printf("IP_MEMORIA: %s\n", unArchivo->IP_MEMORIA);
	unArchivo->PUERTO_MEMORIA = config_get_int_value(config, "PUERTO_MEMORIA");
	printf("PUERTO_MEMORIA: %d\n", unArchivo->PUERTO_MEMORIA);
	unArchivo->IP_FS = config_get_string_value(config, "IP_FS");
	printf("IP_FS: %s\n", unArchivo->IP_FS);
	unArchivo->PUERTO_FS  = config_get_int_value(config, "PUERTO_FS");
	printf("PUERTO_FS: %d\n", unArchivo->PUERTO_FS);
	unArchivo->QUANTUM = config_get_int_value(config, "QUANTUM");
	printf("QUANTUM:%d\n", unArchivo->QUANTUM);
	unArchivo->QUANTUM_SLEEP  = config_get_int_value(config, "QUANTUM_SLEEP");
	printf("QUANTUM_SLEEP:%d\n", unArchivo->QUANTUM_SLEEP );
	unArchivo->ALGORITMO  = config_get_string_value(config, "ALGORITMO");
	printf("ALGORITMO: %s\n", unArchivo->ALGORITMO);
	unArchivo->GRADO_MULTIPROG  = config_get_int_value(config, "GRADO_MULTIPROG");
	printf("GRADO_MULTIPROG:%d\n", unArchivo->GRADO_MULTIPROG);
	unArchivo->STACK_SIZE  = config_get_int_value(config, "STACK_SIZE");
	printf("STACK_SIZE:%d\n", unArchivo->STACK_SIZE);
	unArchivo->SEM_IDS = config_get_array_value(config, "SEM_IDS");
	unArchivo->SEM_INIT  = config_get_array_value(config, "SEM_INIT");
	unArchivo->SHARED_VARS = config_get_array_value(config, "SHARED_VARS");
	while(unArchivo->SEM_IDS[i] != NULL)
	{
		printf("SEM_IDS:%s\n", unArchivo->SEM_IDS[i]);
		i++;
	}
	i=0;
	while(unArchivo->SEM_INIT[i] != NULL)
		{
		unArchivo->SEM_INIT[i] = atoi((int)unArchivo->SEM_INIT[i]);
			printf("SEM_INIT:%d\n", unArchivo->SEM_INIT[i]);
			i++;
	}
	i=0;
		while(unArchivo->SHARED_VARS[i] != NULL)
			{
			printf("SHARED_VARS:%s\n", unArchivo->SHARED_VARS[i]);
			i++;
		}
}


void configuracionCpu(archivoConfigCPU *unArchivo, t_config* config, char *dir){

 		config = config_create(dir);

 		unArchivo->IP_KERNEL = config_get_string_value(config, "IP_KERNEL");
 		printf("IP_KERNEL: %s\n", unArchivo->IP_KERNEL);

 		unArchivo->PUERTO_KERNEL = config_get_int_value(config, "PUERTO_KERNEL");
 		printf("PUERTO_KERNEL: %d\n", unArchivo->PUERTO_KERNEL);

 		unArchivo->IP_MEMORIA = config_get_string_value(config, "IP_MEMORIA");
 		printf("IP_MEMORIA: %s\n", unArchivo->IP_MEMORIA);


 		unArchivo->PUERTO_MEMORIA = config_get_int_value(config, "PUERTO_MEMORIA");
 		printf("PUERTO_MEMORIA: %d\n", unArchivo->PUERTO_MEMORIA);
 }

void configuracionConsola(archivoConfigConsola *unArchivo, t_config* config, char* dir){

			config = config_create(dir);

	 		unArchivo->IP_KERNEL = config_get_string_value(config, "IP_KERNEL");
	 		printf("IP_KERNEL: %s\n", unArchivo->IP_KERNEL);

	 		unArchivo->PUERTO_KERNEL = config_get_int_value(config, "PUERTO_KERNEL");
	 		printf("PUERTO_KERNEL: %d\n", unArchivo->PUERTO_KERNEL);

}

void configuracionFS(archivoConfigFS *unArchivo, t_config* config, char* dir){

	config = config_create(dir);

	 		unArchivo->PUERTO_KERNEL = config_get_int_value(config, "PUERTO_KERNEL");
	 		printf("PUERTO_KERNEL: %d\n", unArchivo->PUERTO_KERNEL);

	 		unArchivo->PUERTO_MONTAJE = config_get_string_value(config, "PUERTO_MONTAJE");
	 		printf("PUERTO_MONTAJE: %s\n", unArchivo->PUERTO_MONTAJE);


}
