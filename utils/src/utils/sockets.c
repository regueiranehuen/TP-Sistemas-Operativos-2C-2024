#include "sockets.h"

#define HANDSHAKE_INIT 1
#define HANDSHAKE_OK 0
#define HANDSHAKE_ERROR -1

int iniciar_servidor(t_log* log, char* puerto) {
    int socket_servidor, error;
    struct addrinfo hints, *servinfo;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    error = getaddrinfo(NULL, puerto, &hints, &servinfo);
    if (error != 0) {
        log_error(log, "Error en getaddrinfo: %s", gai_strerror(error));
        return -1;
    }

    socket_servidor = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if (socket_servidor == -1) {
        log_error(log, "Error al crear el socket: %s", strerror(errno));
        freeaddrinfo(servinfo);
        return -1;
    }

    setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEPORT, &(int){1}, sizeof(int));

    if (bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
        log_error(log, "Error en bind: %s", strerror(errno));
        close(socket_servidor);
        freeaddrinfo(servinfo);
        return -1;
    }

    if (listen(socket_servidor, SOMAXCONN) == -1) {
        log_error(log, "Error en listen: %s", strerror(errno));
        close(socket_servidor);
        freeaddrinfo(servinfo);
        return -1;
    }

    freeaddrinfo(servinfo);
    log_trace(log, "Servidor abierto");
    return socket_servidor;
}

int esperar_cliente(t_log* log, int socket_servidor) {
    struct sockaddr_in dir_cliente;
    socklen_t tam_direccion = sizeof(struct sockaddr_in);
    int socket_cliente = accept(socket_servidor, (struct sockaddr*)&dir_cliente, &tam_direccion);

    if (socket_cliente == -1) {
        log_error(log, "Error en accept: %s", strerror(errno));
        return -1;
    }

    log_info(log, "Cliente conectado desde %s:%d", inet_ntoa(dir_cliente.sin_addr), ntohs(dir_cliente.sin_port));
    return socket_cliente;
}

int servidor_handshake(int socket_cliente, t_log* log) {
    size_t bytes;
    int32_t handshake;
    int32_t resultOk = HANDSHAKE_OK;
    int32_t resultError = HANDSHAKE_ERROR;

    bytes = recv(socket_cliente, &handshake, sizeof(int32_t), MSG_WAITALL);
    if (bytes <= 0) {
        log_info(log, "Error en recv o conexión cerrada por el cliente");
        return -1;
    }

    int32_t respuesta = (handshake == HANDSHAKE_INIT) ? resultOk : resultError;
    bytes = send(socket_cliente, &respuesta, sizeof(int32_t), 0);

    if (bytes == -1) {
        log_error(log, "Error en send: %s", strerror(errno));
        return -1;
    }

    log_info(log, "Handshake realizado correctamente");
    return 0;
}

int cliente_handshake(int socket_cliente, t_log* log) {
    size_t bytes;
    int32_t handshake = HANDSHAKE_INIT;
    int32_t result;

    bytes = send(socket_cliente, &handshake, sizeof(int32_t), 0);
    if (bytes == -1) {
        log_error(log, "Error al enviar handshake al servidor: %s", strerror(errno));
        return -1;
    }

    bytes = recv(socket_cliente, &result, sizeof(int32_t), MSG_WAITALL);
    if (bytes <= 0) {
        log_error(log, "Error al recibir respuesta del servidor: %s", strerror(errno));
        return -1;
    }

    if (result == HANDSHAKE_OK) {
        log_info(log, "Handshake exitoso con el servidor.");
        return 0;
    } else {
        log_error(log, "Handshake fallido con el servidor.");
        return -1;
    }
}

int crear_conexion(t_log* log, char* ip, char* puerto) {
    struct addrinfo hints, *server_info;
    int socket_cliente;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int error = getaddrinfo(ip, puerto, &hints, &server_info);
    if (error != 0) {
        log_error(log, "Error en getaddrinfo: %s", gai_strerror(error));
        return -1;
    }

    socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
    if (socket_cliente == -1) {
        log_error(log, "Error al crear el socket: %s", strerror(errno));
        freeaddrinfo(server_info);
        return -1;
    }

    if (connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1) {
        log_error(log, "Error al conectar con el servidor: %s", strerror(errno));
        close(socket_cliente);
        freeaddrinfo(server_info);
        return -1;
    }

    freeaddrinfo(server_info);
    log_info(log, "Se pudo conectar al servidor");
    return socket_cliente;
}

void liberar_conexion(int socket_cliente) {
    close(socket_cliente);
}

t_config* iniciar_config(char* ruta) {
    t_config* nuevo_config = config_create(ruta);
    if (nuevo_config == NULL) {
        printf("Error: No se pudo crear la configuración en %s\n", ruta);
        return NULL;
    }
    return nuevo_config;
}
