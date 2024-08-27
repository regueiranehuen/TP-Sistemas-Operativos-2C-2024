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
