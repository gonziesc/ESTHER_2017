#include "serializador.h"
typedef enum {
	ok = 0,
	archivo = 1,
	pcb = 2,
	memoria = 3,
	fs = 4,
	kernel = 5,
	cpu = 6,
	consola = 7,
	codigo = 8
} codigosSerializador;

int32_t tamanioInt32 = sizeof(int32_t);


void Serializar(int32_t id, int32_t tamanioArchivo, void* buffer,int32_t  socket){
	void* archivoEmpaquetado = malloc(tamanioArchivo + (tamanioInt32*2));

	switch(id){
		case ok:{
			send(socket, 0, tamanioInt32, 0);
			break;
		}
		case archivo:{
			memcpy(archivoEmpaquetado, &id, tamanioInt32);
			memcpy(archivoEmpaquetado + tamanioInt32, &tamanioArchivo, tamanioInt32);
			memcpy(archivoEmpaquetado + (tamanioInt32*2), buffer, tamanioArchivo);
			send(socket, archivoEmpaquetado, tamanioArchivo, 0);
			break;
		}
		case pcb:{

			break;
		}

		case fs:{
			break;
		}
		case kernel:{
			break;
		}
		case cpu:{
			break;
		}
		case consola:{
			break;
		}
		case codigo:{

		}
		free(archivoEmpaquetado);
	}
}

void Deserializar(int32_t id, void* buffer,int32_t  socket){
	void* archivoDesempaquetado;
	switch(id){
		case ok:{
			printf("Ok");
			archivoDesempaquetado = 0;
			break;
		}
		case archivo:{
			int32_t tamanio;
			recv(socket, &tamanio, tamanioInt32, 0);
			archivoDesempaquetado = malloc(tamanioInt32);
			recv(socket,archivoDesempaquetado , tamanio, 0);
			memcpy(buffer,archivoDesempaquetado,tamanio);
			free(archivoDesempaquetado);

			break;
		}
		case pcb:{

			break;
		}

		case fs:{
			break;
		}
		case kernel:{
			break;
		}
		case cpu:{
			break;
		}
		case consola:{
			break;
		}
		case codigo:{

		}

	}
}



