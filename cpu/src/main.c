#include "includes/main.h"

int main(int argc, char* argv[]) {

    t_log* log;
    t_config* config;
    t_sockets_cpu* sockets;

log = log_create("CPU.log", "tp", true, LOG_LEVEL_TRACE);
config = config_create("CPU.config");

sockets = hilos_cpu(log, config);

liberarMemoria(sockets,log,config);

    return 0;
}

void liberarMemoria(t_sockets_cpu * sockets,t_log* log, t_config* config){

if (sockets == NULL || sockets->socket_cliente == -1 ||
    sockets->socket_servidor == NULL || 
    sockets->socket_servidor->socket_Dispatch == -1 || 
    sockets->socket_servidor->socket_Interrupt == -1) {

    log_info(log, "Error en los sockets de cpu");
    }
    else{
close(sockets->socket_cliente);
close(sockets->socket_servidor->socket_Dispatch);
close(sockets->socket_servidor->socket_Interrupt);
}
   if (sockets != NULL) {
        if (sockets->socket_servidor != NULL) {
            free(sockets->socket_servidor);
        }
        free(sockets);
    }
config_destroy(config);
log_destroy(log);
}