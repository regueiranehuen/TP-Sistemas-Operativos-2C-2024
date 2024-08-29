#include "includes/server.h"

int servidor_memoria_kernel (t_log* log, t_config* config){

char *puerto= config_get_string_value(config,"PUERTO_ESCUCHA");
int socket_servidor,socket_cliente;

socket_servidor = iniciar_servidor(log,puerto);
if (socket_servidor == -1) {
    log_error(log, "Error al iniciar el servidor");
    return -1;
    }
    log_info(log,"Servidor abierto correctamente");
socket_cliente = esperar_cliente(log,socket_servidor);
if (socket_cliente == -1) {
    log_error(log, "Error al esperar cliente");
    close(socket_servidor);
    return -1;
}    
servidor_handshake(socket_cliente,log);


    close(socket_cliente);
	return socket_servidor;
}

int cliente_memoria_filesystem (t_log* log, t_config* config){

char * ip, * puerto;
int socket_cliente, respuesta;

ip = config_get_string_value(config, "IP_FILESYSTEM");
puerto = config_get_string_value(config, "PUERTO_FILESYSTEM");

 // Verificar que ip y puerto no sean NULL
    if (ip == NULL || puerto == NULL) {
        log_info(log, "No se pudo obtener IP o PUERTO de la configuración");
        return -1;
    }

    // Crear conexión
    socket_cliente = crear_conexion(log, ip, puerto);

    if (socket_cliente == -1) {
        log_info(log, "No se pudo crear la conexión");
        return -1;
    }

   respuesta = cliente_handshake(socket_cliente,log);

   if (respuesta == 0){
    log_info(log,"Handshake de Memoria --> Filesystem realizado correctamente");
   }
   else {
    log_error(log, "Handshake de Memoria --> Filesystem tuvo un error");
   }

    return socket_cliente;

}

