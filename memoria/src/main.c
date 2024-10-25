#include "includes/main.h"

int estado_cpu = 1;
t_log *logger;
t_config *config;

int main(int argc, char *argv[])
{

    sockets_memoria *sockets;

    logger = log_create("memoria.log", "tp", true, LOG_LEVEL_TRACE);
    config = config_create("memoria.config");

    inicializar_mutex();
    inicializar_semaforos();

    if (config == NULL)
    {
        log_error(logger, "Error al crear la configuración");
    }

    sockets = hilos_memoria(logger, config);


    // hay que crear un socket que sea servidor de cpu
    int socket_cpu; // ?
    hilo_recibe_cpu(socket_cpu);


    //sem_wait para terminar la ejecucion de memoria
    destruir_mutex();
    destruir_semaforos();

    config_destroy(config);
    log_destroy(logger);
    close(sockets->socket_servidor);
    close(sockets->socket_cliente);
    free(sockets);

    return 0;
}

void hilo_recibe_cpu(int socket_servidor_cpu)
{
    pthread_t hilo_cliente_cpu;
    int resultado = pthread_create(&hilo_cliente_cpu, NULL, recibir_cpu, &socket_servidor_cpu);

    if (resultado != 0)
    {
        log_error(logger, "Error al crear el hilo que recibe a cpu desde memoria");
    }
    pthread_detach(hilo_cliente_cpu);
}