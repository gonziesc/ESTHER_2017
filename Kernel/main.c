#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include <commons/config.h>
#include "abstracciones.c"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ejemplo.c>

void configuracion(archivoConfigKernel *, t_config* , char *);

int main(int argc, char**argv){
	struct sockaddr_in direccionCliente;
	unsigned int tamanoDireccion;
	 int fdmax;
     int newfd;        // descriptor de socket de nueva conexión aceptada
     char buf[256];    // buffer para datos del cliente
     int nbytes;
     int i, j;
    fd_set master;   // conjunto maestro de descriptores de fichero
	fd_set read_fds; // conjunto temporal de descriptores de fichero para select()
	archivoConfigKernel* t_archivoConfig = malloc(sizeof(archivoConfigKernel));
	ejemplo * holaa = malloc(sizeof(ejemplo));
	holaa->chau = 8;

	t_config *config = malloc(sizeof(t_config));
	printf("arranquemos, so, %d \n", holaa->chau);
	configuracion(t_archivoConfig, config, argv[1]);

	 FD_ZERO(&master);    // borra los conjuntos maestro y temporal
	    FD_ZERO(&read_fds);

	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = INADDR_ANY;
	direccionServidor.sin_port = htons(5000);
	 memset(&(direccionServidor.sin_zero), '\0', 8);

		int servidor = socket(AF_INET, SOCK_STREAM, 0);

		int activado = 1;
		setsockopt(servidor, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

		if (bind(servidor, (void*) &direccionServidor, sizeof(direccionServidor)) != 0) {
			perror("Falló el bind");
			return 1;
		}

		printf("Estoy escuchando\n");
		listen(servidor, 100);

		struct sockaddr_in direccionMem;
		direccionMem.sin_family = AF_INET;
		direccionMem.sin_addr.s_addr = inet_addr(t_archivoConfig->IP_MEMORIA);
		direccionMem.sin_port = htons(t_archivoConfig->PUERTO_MEMORIA);

			int clienteMEM = socket(AF_INET, SOCK_STREAM, 0);
			if (connect(clienteMEM, (void*) &direccionMem, sizeof(direccionMem)) != 0) {
				perror("No se pudo conectar");
				return 1;
			}
			struct sockaddr_in direccionFs;
			direccionFs.sin_family = AF_INET;
			direccionFs.sin_addr.s_addr = inet_addr(t_archivoConfig->IP_FS);
			direccionFs.sin_port = htons(t_archivoConfig->PUERTO_FS);

					int clientefs = socket(AF_INET, SOCK_STREAM, 0);
					if (connect(clientefs, (void*) &direccionFs, sizeof(direccionFs)) != 0) {
						perror("No se pudo conectar");
						return 1;
					}



		 FD_SET(servidor, &master);
		        // seguir la pista del descriptor de fichero mayor
		        fdmax = servidor; // por ahora es éste
		 for(;;) {
		            read_fds = master; // cópialo
		            if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
		                perror("select");
		                exit(1);
		            }

		            for(i = 0; i <= fdmax; i++) {
		                           if (FD_ISSET(i, &read_fds)) { // ¡¡tenemos datos!!
		                               if (i == servidor) {
		                                   // gestionar nuevas conexiones
		                            	   tamanoDireccion = sizeof(direccionCliente);
		                                   if ((newfd = accept(servidor, (void*)&direccionCliente,
		                                                                            &tamanoDireccion)) == -1) {
		                                       perror("accept");
		                                   } else {
		                                       FD_SET(newfd, &master); // añadir al conjunto maestro
		                                       if (newfd > fdmax) {    // actualizar el máximo
		                                           fdmax = newfd;
		                                       }
		                                       printf("selectserver: new connection from %s on "
		                                           "socket %d\n", inet_ntoa(direccionCliente.sin_addr), newfd);
		                                   }
		                               } else {
		                                   // gestionar datos de un cliente
		                                   if ((nbytes = recv(i, buf, sizeof(buf), 0)) <= 0) {

		                                       // error o conexión cerrada por el cliente
		                                       if (nbytes == 0) {
		                                           // conexión cerrada
		                                           printf("selectserver: socket %d hung up\n", i);
		                                       } else {
		                                           perror("recv");
		                                       }
		                                       close(i); // bye!
		                                       FD_CLR(i, &master); // eliminar del conjunto maestro
		                                   } else {
		                                	   nbytes[buf] = '\0';
		                                	   printf("Me llegaron %d bytes con %s\n", nbytes, buf);
		                                	   send(clientefs, buf, nbytes, 0);
		                                	   send(clienteMEM, buf, nbytes, 0);
		                                       // tenemos datos de algún cliente
		                                       for(j = 0; j <= fdmax; j++) {
		                                           // ¡enviar a todo el mundo!
		                                           if (FD_ISSET(j, &master)) {
		                                               // excepto al listener y a nosotros mismos
		                                               if (j != servidor && j != i) {
		                                                   if (send(j, buf, nbytes, 0) == -1) {
		                                                       perror("send");
		                                                   }
		                                               }
		                                           }
		                                       }
		                                   }
		                               }
		                           }
		            }
		 }



	return EXIT_SUCCESS;
}
void configuracion(archivoConfigKernel *unArchivo, t_config* config, char *dir){
	int i = 0;
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
