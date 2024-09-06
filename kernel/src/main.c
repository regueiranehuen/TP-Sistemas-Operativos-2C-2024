#include "includes/main.h"

int main(int argc, char *argv[])
{

    t_log *log;
    t_config *config;
    sockets_kernel *sockets;
    t_proceso proceso_inicial;
    char *archivo_pseudocodigo = argv[1];
    int tamanio_proceso = atoi(argv[2]);

    t_queue *cola_new, *cola_ready;
    cola_new = queue_create();
    cola_ready = queue_create();

    log = log_create("kernel.log", "tp", true, LOG_LEVEL_TRACE);
    config = config_create("kernel.config");

    int socket;
    sockets = hilos_kernel(log, config);
    proceso_inicial = iniciar_kernel(archivo_pseudocodigo, tamanio_proceso);
    crear_proceso(cola_new, socket, cola_ready);
    liberar_espacio(log, config, sockets, proceso_inicial);
    return 0;
}

void liberar_espacio(t_log *log, t_config *config, sockets_kernel *sockets, t_proceso proceso_inicial)
{
    config_destroy(config);
    log_destroy(log);
    close(sockets->socket_cliente_memoria);
    close(sockets->sockets_cliente_cpu->socket_Dispatch);
    close(sockets->sockets_cliente_cpu->socket_Interrupt);
    free(sockets);
    free(proceso_inicial.pcb);
    free(proceso_inicial.tcb);
}