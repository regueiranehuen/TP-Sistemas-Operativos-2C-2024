#include "includes/server.h"

void servidor_memoria_kernel (t_log* log, t_config* config){

char *puerto= config_get_string_value(config,"PUERTO_ESCUCHA");
int socket_servidor,socket_cliente;

socket_servidor = iniciar_servidor(log,puerto);
if (socket_servidor == -1) {
    log_error(log, "Error al iniciar el servidor");
    return;
    }
    log_info(log,"Servidor abierto correctamnete");
socket_cliente = esperar_cliente(log,socket_servidor);
if (socket_cliente == -1) {
    log_error(log, "Error al esperar cliente");
    close(socket_servidor);
    return;
}    
t_list* lista;
	while (1) {
		int cod_op = recibir_operacion(socket_cliente);
		switch (cod_op) {
		case MENSAJE:
			recibir_mensaje(socket_cliente);
			break;
		case PAQUETE:
			lista = recibir_paquete(socket_cliente);
			log_info(log, "Me llegaron los siguientes valores:\n");
			list_iterate(lista, (void*) iterator);
			break;
		case -1:
			log_error(log, "el cliente se desconecto. Terminando servidor");
			return EXIT_FAILURE;
		default:
			log_warning(log,"Operacion desconocida. No quieras meter la pata");
			break;
		}
	}


    close(socket_cliente);
    close(socket_servidor);
    return EXIT_SUCCESS;
}
