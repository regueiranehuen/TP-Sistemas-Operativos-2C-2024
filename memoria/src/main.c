#include "includes/main.h"

int estado_cpu = 1;
t_log* logger;
t_config* config;

int main(int argc, char* argv[]) {

sockets_memoria* sockets;

logger = log_create("memoria.log", "tp", true, LOG_LEVEL_TRACE);
config= config_create("memoria.config");

if (config == NULL) {
    log_error(logger, "Error al crear la configuración");
}

sockets = hilos_memoria(logger,config);



    config_destroy(config);
	log_destroy(logger);
    close(sockets->socket_servidor);
    close(sockets->socket_cliente);
    free(sockets);

    return 0;
}