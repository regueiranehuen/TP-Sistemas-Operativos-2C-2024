#include "includes/main.h"

    int estado_kernel = 1;

int main(int argc, char *argv[])
{

    char *archivo_pseudocodigo = argv[1];
    int tamanio_proceso = atoi(argv[2]);
    
    logger = log_create("kernel.log", "tp", true, LOG_LEVEL_TRACE);
    config = config_create("kernel.config");

    inicializar_estados();
    inicializar_semaforos();
    inicializar_mutex();

    sockets = hilos_kernel(logger, config);
    iniciar_kernel(archivo_pseudocodigo, tamanio_proceso);
    sem_wait(&sem_fin_kernel);
    estado_kernel = 0;
    liberar_espacio(logger, config, sockets);
    return 0;
}

void liberar_espacio(t_log *logger, t_config *config, sockets_kernel *sockets)
{
    config_destroy(config);
    log_destroy(logger);
    close(sockets->socket_cliente_memoria);
    close(sockets->sockets_cliente_cpu->socket_Dispatch);
    close(sockets->sockets_cliente_cpu->socket_Interrupt);
    destruir_estados();
    destruir_semaforos();
    destruir_mutex();
    free(sockets);
}
