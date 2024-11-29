#include "auxiliares.h"

void inicializar_semaforos(){
    sem_init(&sem_ok_o_interrupcion,0,0);
    sem_init(&sem_finalizacion_cpu,0,0);
    sem_init(&sem_ciclo_instruccion,0,0);
    sem_init(&sem_socket_cerrado,0,0);

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
    sem_destroy(&sem_finalizacion_cpu);
    sem_destroy(&sem_finalizacion_cpu);
    sem_destroy(&sem_ciclo_instruccion);
    sem_destroy(&sem_socket_cerrado);

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
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(sockets_cpu->socket_servidor->socket_cliente_Dispatch,&readfds);

    int actividad = select(sockets_cpu->socket_servidor->socket_cliente_Dispatch + 1,&readfds,NULL,NULL,NULL);

    if (actividad < 0)
    {
        log_error(log_cpu,"Error en select (socket dispatch)");
        exit(EXIT_FAILURE);
    }

    // Si se detecta actividad en el socket de dispatch
    if (FD_ISSET(sockets_cpu->socket_servidor->socket_cliente_Dispatch, &readfds))
    {
        log_info(log_cpu,"Se detectó actividad en el socket de dispatch");
        sem_wait(&sem_socket_cerrado);
    }

    


    close(sockets_cpu->socket_servidor->socket_servidor_Dispatch);
    close(sockets_cpu->socket_servidor->socket_cliente_Dispatch);
    
    close(sockets_cpu->socket_servidor->socket_servidor_Interrupt);
    close(sockets_cpu->socket_servidor->socket_cliente_Interrupt);
    
    close(sockets_cpu->socket_memoria);

    destruir_mutex();
    destruir_semaforos();
    free(sockets);
    free(sockets_cpu);
    log_debug(log_cpu, "Estructuras liberadas. CPU TERMINADO");
    config_destroy(config);
    log_destroy(log_cpu);
}




t_instruccion *recepcionar_instruccion(t_paquete *paquete)
{
    t_instruccion *instruccion_nueva = malloc(sizeof(t_instruccion));

    void *stream = paquete->buffer->stream;
    // int cantParam = 0;
    // memcpy(&(cantParam), stream, sizeof(int));
    //stream += sizeof(int);

    //log_info(log_cpu, "Cantidad parametros:%d", cantParam);
    int l1 = 0;
    int l2 = 0;
    int l3 = 0;
    int l4 = 0;

    memcpy(&l1, stream, sizeof(int));
    stream += sizeof(int);
    instruccion_nueva->parametros1 = malloc(l1);
    memcpy(instruccion_nueva->parametros1, stream, l1);

    stream += l1;
    memcpy(&l2, stream, sizeof(int));
    stream += sizeof(int);
    instruccion_nueva->parametros2 = malloc(l2);
    memcpy(instruccion_nueva->parametros2, stream, l2);

    stream += l2;
    memcpy(&l3, stream, sizeof(int));
    stream += sizeof(int);
    instruccion_nueva->parametros3= malloc(l3);
    memcpy(instruccion_nueva->parametros3, stream, l3);

    stream += l3;
    memcpy(&l4, stream, sizeof(int));
    stream += sizeof(int);
    instruccion_nueva->parametros4 = malloc(l4);
    memcpy(instruccion_nueva->parametros4, stream, l4);

    stream += l4;

    // DUMP_MEMORY, THREAD_EXIT Y PROCESS_EXIT NO LLEVAN PARÁMETROS

    eliminar_paquete(paquete);
    return instruccion_nueva;
}