#include "includes/main.h"

    int estado_kernel = 1;

int main(int argc, char *argv[])
{

    if (argc <= 1 || argc > 4)
    {
        
        printf("Ingrese ./bin/kernel <archivo_pseudocodigo> <tamaño proceso> <path del config>");
        
        return -1;
    }

    char* archivo_pseudocodigo = argv[1];
    int tamanio_proceso = atoi(argv[2]);
    char* path_config = argv[3];

    config = config_create(path_config);
    t_log_level log_level_int = log_level(config);
    logger = log_create("kernel.log", "tp", true, log_level_int);

    inicializar_estados();
    inicializar_semaforos();
    inicializar_mutex();

    sockets = hilos_kernel(logger, config);
    iniciar_kernel(archivo_pseudocodigo, tamanio_proceso);
    sem_wait(&sem_fin_kernel);

    pthread_mutex_lock(&mutex_estado_kernel);
    estado_kernel = 0;
    pthread_mutex_unlock(&mutex_estado_kernel);
    liberar_espacio(logger, config, sockets);
    return 0;
}

void liberar_espacio(t_log *logger, t_config *config, sockets_kernel *sockets)
{
    
    char*algoritmo=config_get_string_value(config,"ALGORITMO_PLANIFICACION");
    
    shutdown(sockets->sockets_cliente_cpu->socket_Dispatch, SHUT_RDWR);
    shutdown(sockets->sockets_cliente_cpu->socket_Interrupt, SHUT_RDWR);  

    free(sockets->sockets_cliente_cpu);

    sem_post(&semaforo_cola_new_procesos);
    sem_post(&semaforo_cola_exit_procesos);
    sem_post(&semaforo_cola_exit_hilos);
    sem_post(&sem_seguir_o_frenar);
    sem_post(&sem_cola_IO);
    sem_post(&sem_lista_prioridades);
    sem_post(&semaforo_cola_ready);
    sem_post(&sem_ciclo_nuevo);
    sem_post(&sem_desalojado);

    int cant_hilos=7;

    if (strings_iguales(algoritmo,"PRIORIDADES")){
        sem_post(&sem_lista_prioridades);
        cant_hilos+=1;
    }
    for (int i = 0; i<cant_hilos;i++){
        sem_wait(&sem_termina_hilo);
        pthread_mutex_lock(&mutex_log);
        log_info(logger,"Hilos contados: %d",i+1);
        pthread_mutex_unlock(&mutex_log);
    }
    

    /*pthread_cancel(hilo_exit_procesos);
    pthread_join(hilo_exit_procesos,NULL);
    pthread_cancel(hilo_hilos_exit);
    pthread_join(hilo_hilos_exit,NULL);
    pthread_cancel(hilo_new_ready_procesos);
    pthread_join(hilo_new_ready_procesos,NULL);
    pthread_cancel(hilo_plani_largo_plazo);
    pthread_join(hilo_plani_largo_plazo,NULL);
    pthread_cancel(hilo_cortar_modulos);
    pthread_join(hilo_cortar_modulos,NULL);
    pthread_cancel(hilo_atender_interrupt);
    pthread_join(hilo_atender_interrupt,NULL);
    pthread_cancel(hilo_atender_syscall);
    pthread_join(hilo_atender_syscall,NULL);
    pthread_cancel(hilo_ready_exec);
    pthread_join(hilo_ready_exec,NULL);*/
    
    
    
    close(sockets->socket_cliente_memoria);
    free(sockets);


    destruir_estados(); 
    destruir_semaforos();
    destruir_mutex();
    pthread_mutex_lock(&mutex_log);
    log_debug(logger, "Estructuras liberadas. KERNEL TERMINADO");
    log_destroy(logger);
    pthread_mutex_unlock(&mutex_log);
    config_destroy(config);
}
