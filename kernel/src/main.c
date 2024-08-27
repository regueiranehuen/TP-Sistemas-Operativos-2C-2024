#include "includes/main.h"

int main(int argc, char* argv[]) {

t_log* log;
t_config* config;
int socket_cliente_memoria, socket_cpu_dispatch, socket_cpu_interrupt;
t_socket_cpu sockets;

log= log_create("kernel.log", "tp", true, LOG_LEVEL_TRACE);
config= config_create("kernel.config");

socket_cliente_memoria = cliente_Memoria_Kernel(log,config);
sockets= cliente_CPU_Kernel(log,config);
socket_cpu_dispatch=sockets.socket_Dispatch;
socket_cpu_interrupt=sockets.socket_Interrupt;


    config_destroy(config);
	log_destroy(log);
    close(socket_cliente_memoria);
    close(socket_cpu_dispatch);
    close(socket_cpu_interrupt);

    return 0;
}
