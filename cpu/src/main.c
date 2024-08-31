#include "includes/main.h"

int main(int argc, char* argv[]) {

    t_log* log;
    t_config* config;
    t_sockets_cpu* sockets;

log = log_create("CPU.log", "tp", true, LOG_LEVEL_TRACE);
config = config_create("CPU.config");

sockets = hilos_cpu(log, config);

close(sockets->socket_cliente->socket_Dispatch);
close(sockets->socket_cliente->socket_Interrupt);
close(sockets->socket_servidor->socket_Dispatch);
close(sockets->socket_servidor->socket_Interrupt);
config_destroy(config);
log_destroy(log);

    return 0;
}
