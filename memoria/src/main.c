#include "includes/main.h"

int main(int argc, char* argv[]) {

t_log* log;
t_config* config;
sockets_memoria* sockets;

log = log_create("memoria.log", "tp", true, LOG_LEVEL_TRACE);
config= config_create("memoria.config");

if (config == NULL) {
    log_error(log, "Error al crear la configuración");
}

sockets = hilos_memoria(log,config);



    config_destroy(config);
	log_destroy(log);
    close(sockets->socket_servidor);
    close(sockets->socket_cliente);
    free(sockets);

    return 0;
}