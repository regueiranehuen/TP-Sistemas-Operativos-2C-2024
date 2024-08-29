#include "utils_server.h"

t_log* logger;

int iniciar_servidor(char*puerto, struct addrinfo * hints_param)
{

	struct addrinfo hints, *servinfo, *p;


	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(NULL, puerto, &hints, &servinfo);

	// Creamos el socket de escucha del servidor, que en este caso es fd_escucha

	int fd_escucha = socket(servinfo->ai_family,
                        servinfo->ai_socktype,
                        servinfo->ai_protocol);


	// Asociamos el socket a un puerto
	setsockopt(fd_escucha, SOL_SOCKET, SO_REUSEPORT, &(int){1}, sizeof(int));

	bind(fd_escucha, servinfo->ai_addr, servinfo->ai_addrlen);

	*hints_param=hints;


	freeaddrinfo(servinfo);

	return fd_escucha;
}

int esperar_cliente(int socket_servidor)
{

	listen(socket_servidor, SOMAXCONN);

	log_info(logger, "Esperando clientes, el socket esta listo!!!!");

	int socket_conexion = accept(socket_servidor, NULL, NULL);

	return socket_conexion;
}

int recibir_operacion(int socket_conexion)
{
	int cod_op;
	if(recv(socket_conexion, &cod_op, sizeof(int), MSG_WAITALL) > 0)
		return cod_op;
	else
	{
		close(socket_conexion);
		return -1;
	}
}

void* recibir_buffer(int* size, int socket_conexion)
{
	void * buffer;

	recv(socket_conexion, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_conexion, buffer, *size, MSG_WAITALL);

	return buffer;
}

void recibir_mensaje(int socket_conexion)
{
	int size;
	char* buffer = recibir_buffer(&size, socket_conexion);
	log_info(logger, "Me llego el mensaje %s", buffer);
	free(buffer);
}

t_list* recibir_paquete(int socket_conexion)
{
	int size;
	int desplazamiento = 0;
	void * buffer;
	t_list* valores = list_create();
	int tamanio;

	buffer = recibir_buffer(&size, socket_conexion);
	while(desplazamiento < size)
	{
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		char* valor = malloc(tamanio);
		memcpy(valor, buffer+desplazamiento, tamanio);
		desplazamiento+=tamanio;
		list_add(valores, valor);
	}
	free(buffer);
	return valores;
}
