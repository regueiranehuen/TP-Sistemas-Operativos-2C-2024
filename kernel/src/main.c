#include "includes/main.h"

    int estado_kernel = 1;

int main(int argc, char *argv[])
{

    t_log *log;
    t_config *config;
    sockets_kernel *sockets;
    char *archivo_pseudocodigo = argv[1];
    int tamanio_proceso = atoi(argv[2]);
    
    log = log_create("kernel.log", "tp", true, LOG_LEVEL_TRACE);
    config = config_create("kernel.config");

    sockets = hilos_kernel(log, config);
    iniciar_kernel(archivo_pseudocodigo, tamanio_proceso);
    estado_kernel = 0;
    liberar_espacio(log, config, sockets);
    return 0;
}

void liberar_espacio(t_log *log, t_config *config, sockets_kernel *sockets)
{
    config_destroy(config);
    log_destroy(log);
    close(sockets->socket_cliente_memoria);
    close(sockets->sockets_cliente_cpu->socket_Dispatch);
    close(sockets->sockets_cliente_cpu->socket_Interrupt);
    free(sockets);
}
