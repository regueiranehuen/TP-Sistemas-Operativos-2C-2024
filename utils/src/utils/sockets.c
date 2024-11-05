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
    
    bytes = recv(socket_cliente, &result, sizeof(int32_t), 0);

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

//ALGUNO DE TODOS LOS CAMBIOS HACE QUE ESTO DE ERROR XD
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
    log_info(log,"Error al crear el socket");
    return socket_cliente;
}
    bool conexion = false;
    int i = 1;
    while(conexion != true && i<15){//15 segundos de espera para que el servidor haya arrancado
    if(connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == 0){
            conexion = true;
            printf("Kernel se conecto");
            freeaddrinfo(server_info); // Liberar la memoria asignada por getaddrinfo()
            return socket_cliente;
    }
    else{
        log_info(log,"No se pudo conectar al servidor, esperando: %d segundos", i*1);
        sleep(1*i);
        i++;
    }
    }
    freeaddrinfo(server_info); // Liberar la memoria asignada por getaddrinfo()

    return -1;
}