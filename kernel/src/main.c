#include "includes/main.h"

    int estado_kernel = 1;

int main(int argc, char *argv[])
{
    config = config_create("kernel.config");
    t_log_level log_level_int = log_level(config);
    logger = log_create("kernel.log", "tp", true, log_level_int);
    
    if(argc <=1 || argc >3){
    log_info(logger,"Ingrese ./bin/kernel <archivo_pseudocodigo> <tamaño proceso>");
    log_destroy(logger);
    return -1;
    }

    char* archivo_pseudocodigo = argv[1];
    int tamanio_proceso = atoi(argv[2]);
    //char *archivo_pseudocodigo = "pseudocodigo";//argv[1];
    //int tamanio_proceso = 64;//atoi(argv[2]);

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
    
    char* algoritmo = config_get_string_value(config,"ALGORITMO_PLANIFICACION");

    send_terminar_ejecucion(sockets->sockets_cliente_cpu->socket_Interrupt);
    sem_wait(&sem_modulo_terminado);

    close(sockets->socket_cliente_memoria);
    close(sockets->sockets_cliente_cpu->socket_Dispatch);
    close(sockets->sockets_cliente_cpu->socket_Interrupt);    

    sem_post(&semaforo_cola_new_procesos);
    sem_post(&semaforo_cola_exit_procesos);
    sem_post(&semaforo_cola_exit_hilos);
    sem_post(&sem_seguir_o_frenar);
    sem_post(&sem_cola_IO);
    if (strings_iguales(algoritmo,"PRIORIDADES")){
        sem_post(&sem_lista_prioridades);
    }
    
    int cantHilos = 7;
    if (strings_iguales(algoritmo,"PRIORIDADES")){
        cantHilos +=1;
    }
    

    for (int i = 1; i < cantHilos; i++){
        log_info(logger,"cant hilos que terminaron: %d",i);
        sem_wait(&sem_termina_hilo);
    }


    destruir_estados(); 
    destruir_semaforos();
    destruir_mutex();
    free(sockets);
    log_debug(logger, "Estructuras liberadas. KERNEL TERMINADO");
    log_destroy(logger);
    config_destroy(config);
}
