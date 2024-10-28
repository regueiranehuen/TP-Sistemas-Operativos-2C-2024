#include "includes/main.h"

int estado_cpu = 1;
t_log *logger;
t_config *config;

int main(int argc, char *argv[])
{

    sockets_memoria *sockets = malloc(sizeof(sockets_memoria));
    sockets_iniciales = malloc(sizeof(sockets_memoria));

    logger = log_create("memoria.log", "tp", true, LOG_LEVEL_TRACE);
    config = config_create("memoria.config");

    inicializar_mutex();
    inicializar_semaforos();
    inicializar_estructuras();

    if (config == NULL)
    {
        log_error(logger, "Error al crear la configuración");
        return -1;
    }

    sockets = hilos_memoria(logger, config);

    sem_wait(&sem_conexion_iniciales); //esperar a que se haga la conexion con cpu y kernel
    //printf("empieza memoria \n");


    //hilo_recibe_cpu();
    
    sem_wait(&sem_fin_memoria);
    //sem_wait para terminar la ejecucion de memoria
    
    destruir_mutex();
    destruir_semaforos();

    config_destroy(config);
    log_destroy(logger);
    close(sockets->socket_servidor);
    close(sockets->socket_cliente);
    close(sockets_iniciales->socket_cpu);
    close(sockets_iniciales->socket_kernel);
    free(sockets_iniciales);
    free(sockets);

    return 0;
}

void hilo_recibe_cpu()
{
    pthread_t hilo_cliente_cpu;
    int resultado = pthread_create(&hilo_cliente_cpu, NULL, recibir_cpu, NULL);

    if (resultado != 0)
    {
        log_error(logger, "Error al crear el hilo que recibe a cpu desde memoria");
    }
    pthread_detach(hilo_cliente_cpu);
}