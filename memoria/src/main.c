#include "includes/main.h"

int main(int argc, char* argv[]) {

t_log* log;
t_config* config;
int socket_servidor, socket_cliente;

log = log_create("memoria.log", "tp", true, LOG_LEVEL_TRACE);
config= config_create("memoria.config");

if (config == NULL) {
    log_error(log, "Error al crear la configuración");
}

socket_cliente= cliente_memoria_filesystem(log,config);
socket_servidor=servidor_memoria_kernel(log,config);

    config_destroy(config);
	log_destroy(log);
    close(socket_servidor);
    close(socket_cliente);

    return 0;
}