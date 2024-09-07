#include "includes/main.h"

int main(int argc, char* argv[]) {

t_log* log;
t_config* config;
sockets_kernel* sockets;

log= log_create("kernel.log", "tp", true, LOG_LEVEL_TRACE);
config= config_create("kernel.config");

sockets = hilos_kernel(log,config);

    config_destroy(config);
	log_destroy(log);
    close(sockets->socket_cliente_memoria);
    close(sockets->sockets_cliente_cpu->socket_Dispatch);
    close(sockets->sockets_cliente_cpu->socket_Interrupt);
    free(sockets);

    return 0;
}
