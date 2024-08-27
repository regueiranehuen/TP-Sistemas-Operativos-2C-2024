#include "includes/main.h"

int main(int argc, char* argv[]) {

    t_log* log;
    t_config* config;
    t_socket_cpu sockets;
    int socket_interrupt, socket_Dispatch;

log = log_create("CPU.log", "tp", true, LOG_LEVEL_TRACE);
config = config_create("CPU.config");

sockets = servidor_CPU_Kernel(log, config);
socket_Dispatch = sockets.socket_Dispatch;
socket_interrupt = sockets.socket_Interrupt;

close(socket_Dispatch);
close(socket_interrupt);
config_destroy(config);
log_destroy(log);

    return 0;
}
