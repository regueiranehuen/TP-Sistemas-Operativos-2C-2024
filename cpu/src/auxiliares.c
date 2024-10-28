#include "auxiliares.h"

void inicializar_semaforos(){
    sem_init(&sem_syscall_finalizada,0,0);
    sem_init(&sem_finalizacion_cpu,0,0);
    sem_init(&sem_ciclo_instruccion,0,0);
}

void inicializar_estructuras() {
    log_cpu = log_create("CPU.log", "tp", true, LOG_LEVEL_TRACE);
    leer_config("CPU.config");
    
    
    sockets_cpu = malloc(sizeof(t_sockets_cpu));


    log_info(log_cpu, "Estructuras inicializadas");
}


void inicializar_mutex(){
    pthread_mutex_init(&mutex_contextos_exec,NULL);
    pthread_mutex_init(&mutex_interrupt,NULL);
}



void destruir_mutex(){
    pthread_mutex_destroy(&mutex_contextos_exec);
    pthread_mutex_destroy(&mutex_interrupt);
}

void destruir_semaforos(){
    sem_destroy(&sem_syscall_finalizada);
    sem_destroy(&sem_finalizacion_cpu);
    sem_destroy(&sem_ciclo_instruccion);
}

void liberarMemoria(t_sockets_cpu * sockets,t_log* log, t_config* config){

    if (sockets == NULL || sockets->socket_memoria == -1 ||
        sockets->socket_servidor == NULL || 
        sockets->socket_servidor->socket_cliente_Dispatch == -1 || 
        sockets->socket_servidor->socket_cliente_Interrupt == -1) {

        log_info(log, "Error en los sockets de cpu");
        }
        else{
    close(sockets->socket_memoria);
    close(sockets->socket_servidor->socket_servidor_Dispatch);
    close(sockets->socket_servidor->socket_servidor_Interrupt);
    close(sockets->socket_servidor->socket_cliente_Dispatch);
    close(sockets->socket_servidor->socket_cliente_Interrupt);

    }
    if (sockets != NULL) {
            if (sockets->socket_servidor != NULL) {
                free(sockets->socket_servidor);
            }
            free(sockets);
        }
    config_destroy(config);
    log_destroy(log);
}

void terminar_programa() {
    liberarMemoria(sockets_cpu, log_cpu, config); 
    destruir_mutex();
    destruir_semaforos();
    log_info(log_cpu, "Estructuras liberadas");
}  