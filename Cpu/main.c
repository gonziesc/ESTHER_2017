#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include <commons/config.h>
#include <sys/socket.h>
#include <arpa/inet.h>




int main(int argc, char**argv){
	//hacer config
		//configuracion(t_archivoConfig, config, argv[1]);
		char *ipKernel = "127.0.0.1";
		int puertoKernel = 5000;

		struct sockaddr_in direccionKernel;
		direccionKernel.sin_family = AF_INET;
		direccionKernel.sin_addr.s_addr = inet_addr(ipKernel);
		direccionKernel.sin_port = htons(puertoKernel);

			int cliente = socket(AF_INET, SOCK_STREAM, 0);
			if (connect(cliente, (void*) &direccionKernel, sizeof(direccionKernel)) != 0) {
				perror("No se pudo conectar");
				return 1;
			}
			while(1){};

		return EXIT_SUCCESS;
}

//void configuracion(archivoConfigKernel *unArchivo, t_config* config, char dir[]){


//}
