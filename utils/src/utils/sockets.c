#include "includes/sockets.h"

int iniciar_servidor(t_log* log,char* puerto)
{
// Creación del socket de escucha

	int error;
	int socket_servidor;

	struct addrinfo hints, *servinfo; //*p;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	error=getaddrinfo(NULL, puerto, &hints, &servinfo);
	if (error != 0) {
        log_error(log, "Error en getaddrinfo: %s", gai_strerror(error));
        return -1;
    }

	socket_servidor= socket(servinfo->ai_family,
                        servinfo->ai_socktype,
                        servinfo->ai_protocol);

//bind y listen

	setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEPORT, &(int){1}, sizeof(int));

	if (bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
    log_error(log, "Error en bind: %s", strerror(errno));
    freeaddrinfo(servinfo);
    return -1;
}

if (listen(socket_servidor, SOMAXCONN) == -1) {
    log_error(log, "Error en listen: %s", strerror(errno));
    freeaddrinfo(servinfo);
    return -1;
}

freeaddrinfo(servinfo);

if (socket_servidor == -1){
    log_info (log,"Error en la creación del socket_servidor");
    return -1;
}
	log_trace(log, "Servidor abierto");

	return socket_servidor;
}

int esperar_cliente(t_log* log,int socket_servidor)
{
//accept
	int socket_cliente;
	socket_cliente= accept(socket_servidor, NULL, NULL);
if (socket_cliente == -1){
    log_info(log,"Error en la conexion con el cliente");
    return -1;
}
	log_info(log, "Se conecto un cliente!");

	return socket_cliente;
}

int servidor_handshake(int socket_cliente, t_log* log) {
    size_t bytes;
    int32_t handshake;
    int32_t resultOk = 0;
    int32_t resultError = -1;

    // Recibir handshake
    bytes = recv(socket_cliente, &handshake, sizeof(int32_t), MSG_WAITALL);
    if (bytes <= 0) {
		log_info(log,"Error en recv o conexion cerrada por el cliente");
      
        return -1;
    }

    // Verificar valor del handshake
    if (handshake == 1) {
        // Enviar respuesta OK
        bytes = send(socket_cliente, &resultOk, sizeof(int32_t), 0);
    } else {
        // Enviar respuesta de error
        bytes = send(socket_cliente, &resultError, sizeof(int32_t), 0);
    }

    // Verificar si send fue exitoso
    if (bytes == -1) {
        // Error en send
        return -1;
    }
	log_info(log,"Hanshake realizado correctamente");

    return 0;
}

int cliente_handshake(int socket_cliente, t_log* log) {
    size_t bytes;
    int32_t handshake = 1;
    int32_t result;
    int32_t resultOk = 0;

    // Enviar handshake al servidor
    bytes = send(socket_cliente, &handshake, sizeof(int32_t), 0);
    if (bytes == -1) {
        log_error(log, "Error al enviar handshake al servidor: %s", strerror(errno));
        return -1;
    }

    // Recibir respuesta del servidor
    bytes = recv(socket_cliente, &result, sizeof(int32_t), MSG_WAITALL);
    if (bytes == -1) {
        log_error(log, "Error al recibir respuesta del servidor: %s", strerror(errno));
        return -1;
    }

    // Verificar la respuesta del servidor
    if (result == resultOk) {
        log_info(log, "Handshake exitoso con el servidor.");
        return 0; // Handshake OK
    } else {
        log_error(log, "Handshake fallido con el servidor.");
        return -1; // Handshake ERROR
    }
}


int crear_conexion(t_log* log,char *ip, char* puerto) {

//crear socket de conexion
    struct addrinfo hints;
    struct addrinfo *server_info;
    int socket_cliente;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    getaddrinfo(ip, puerto, &hints, &server_info);

//conexion con el servidor

socket_cliente = socket(server_info->ai_family,
                         server_info->ai_socktype,
                         server_info->ai_protocol);
if (socket_cliente == -1){
    log_info(log,"Error al conectarse con el servidor");
    return socket_cliente;
}
	log_info(log,"Se pudo conectar al servidor");

    connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen);

    freeaddrinfo(server_info); // Liberar la memoria asignada por getaddrinfo()

    return socket_cliente;
}


/*

int crear_conexion(t_log* log,char *ip, char* puerto) {

//crear socket de conexion
    struct addrinfo hints;
    struct addrinfo *server_info;
    struct addrinfo *p;
    int socket_cliente = -1; // Inicializar en un valor inválido
    int err;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0; // No usar AI_PASSIVE si estamos conectando a una IP específica

    err = getaddrinfo(ip, puerto, &hints, &server_info);
    if (err != 0) {
        log_info(log, "Error en getaddrinfo: %s\n", gai_strerror(err));
        return -1; // Error al obtener información del servidor
    }
//conexion con el servidor

    // Iterar a través de la lista de resultados y tratar de conectarse
    for (p = server_info; p != NULL; p = p->ai_next) {
        socket_cliente = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (socket_cliente == -1) {
            perror("Error al crear el socket");
            continue; // Intentar con la siguiente dirección
        }

        if (connect(socket_cliente, p->ai_addr, p->ai_addrlen) == -1) {
            close(socket_cliente); // Cerrar el socket en caso de error
            perror("Error al conectar");
            continue; // Intentar con la siguiente dirección
        }

        break; // Si llegamos aquí, la conexion fue exitosa
    }
	log_info(log,"Se pudo conectar al servidor");

    freeaddrinfo(server_info); // Liberar la memoria asignada por getaddrinfo()

    if (p == NULL) {
        log_info(log, "No se pudo conectar a ninguna de las direcciones proporcionadas\n");
        return -1; // No se pudo conectar
    }

    return socket_cliente;
}

void* recibir_buffer(int* size, int socket_cliente)
{
	void * buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

void recibir_mensaje(t_log* log,int socket_cliente)
{
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	log_info(log, "Me llego el mensaje %s", buffer);
	free(buffer);
}

int recibir_operacion(int socket_cliente)
{
	int cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}

t_list* recibir_paquete(int socket_cliente)
{
	int size;
	int desplazamiento = 0;
	void * buffer;
	t_list* valores = list_create();
	int tamanio;

	buffer = recibir_buffer(&size, socket_cliente);
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

void enviar_mensaje(char* mensaje, int socket_cliente)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = MENSAJE;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2*sizeof(int);

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}
void eliminar_paquete(t_paquete* paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

void crear_buffer(t_paquete* paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

void* serializar_paquete(t_paquete* paquete, int bytes)
{
	void * magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;

	return magic;
}
*/
