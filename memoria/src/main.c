#include "includes/main.h"

int estado_memoria = 1;
t_log *logger;
t_config *config;

int main(int argc, char *argv[])
{
    
    if (argc <= 1 || argc > 2)
    {
        
        printf("Ingrese ./bin/memoria <path del config>");

        return -1;
    }

    sockets_memoria *sockets;
    sockets_iniciales = malloc(sizeof(sockets_memoria));
    char* path_config = argv[1];
    config = config_create(path_config);
    t_log_level log_level_int = log_level(config);

    

    logger = log_create("memoria.log", "tp", true, log_level_int);
    pthread_mutex_lock(&mutex_logs);
    log_info(logger,"Retardo respuesta: %s",config_get_string_value(config,"RETARDO_RESPUESTA"));
    pthread_mutex_unlock(&mutex_logs);

    inicializar_Memoria(config);
    inicializar_mutex();
    inicializar_semaforos();
    inicializar_estructuras();

    if (config == NULL)
    {
        pthread_mutex_lock(&mutex_logs);
        log_error(logger, "Error al crear la configuración");
        pthread_mutex_unlock(&mutex_logs);
        return -1;
    }

    sockets = hilos_memoria(logger, config);

    sem_wait(&sem_conexion_iniciales); //esperar a que se haga la conexion con cpu y kernel
    sem_wait(&sem_conexion_iniciales);

    hilo_recibe_cpu();

    sem_wait(&sem_fin_memoria);
    //sem_wait para terminar la ejecucion de memoria
    pthread_mutex_lock(&mutex_estado_memoria);
    estado_memoria = 0;
    pthread_mutex_unlock(&mutex_estado_memoria);

    sem_post(&sem_conexion_hecha);
    sem_wait(&sem_termina_hilo);

    eliminar_estructuras();

    destruir_mutex();
    destruir_semaforos();
    
    config_destroy(config);
    
    close(sockets->socket_servidor);
    close(sockets->socket_cliente);
    close(sockets_iniciales->socket_cpu);
    close(sockets_iniciales->socket_kernel);
    free(sockets_iniciales);
    free(sockets);
    pthread_mutex_lock(&mutex_logs);
    log_debug(logger, "Estructuras liberadas. MEMORIA TERMINADO");
    pthread_mutex_unlock(&mutex_logs);
    log_destroy(logger);
    return 0;
}

void hilo_recibe_cpu()
{
    pthread_t hilo_cliente_cpu;
    int resultado = pthread_create(&hilo_cliente_cpu, NULL, recibir_cpu, NULL);

    if (resultado != 0)
    {
        pthread_mutex_lock(&mutex_logs);
        log_error(logger, "Error al crear el hilo que recibe a cpu desde memoria");
        pthread_mutex_unlock(&mutex_logs);
    }
    pthread_detach(hilo_cliente_cpu);
}