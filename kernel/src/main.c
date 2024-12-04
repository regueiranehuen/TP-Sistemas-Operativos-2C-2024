#include "includes/main.h"

    int estado_kernel = 1;

int main(int argc, char *argv[])
{
    config = config_create("kernel.config");
    t_log_level log_level_int = log_level(config);
    logger = log_create("kernel.log", "tp", true, log_level_int);
    
    /*if(argc <=1 || argc >3){
    log_info(logger,"Ingrese ./bin/kernel <archivo_pseudocodigo> <tamaño proceso>");
    log_destroy(logger);
    return -1;
    }*/
    
    //char* archivo_pseudocodigo = argv[1];
    //int tamanio_proceso = atoi(argv[2]);
    char *archivo_pseudocodigo = "RECURSOS_MUTEX_PROC";//argv[1];
    int tamanio_proceso = 32;//atoi(argv[2]);

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
    
    char*algoritmo=config_get_string_value(config,"ALGORITMO_PLANIFICACION");
    
    shutdown(sockets->sockets_cliente_cpu->socket_Dispatch, SHUT_RDWR);
    log_info(logger,"shutdown1");
    shutdown(sockets->sockets_cliente_cpu->socket_Interrupt, SHUT_RDWR);  
    log_info(logger,"shutdown2");

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
        log_info(logger,"Hilos contados: %d",i+1);
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
    log_debug(logger, "Estructuras liberadas. KERNEL TERMINADO");
    log_destroy(logger);
    config_destroy(config);
}
