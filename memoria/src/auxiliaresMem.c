#include "includes/auxiliaresMem.h"

void inicializar_semaforos(){
    sem_init(&sem_conexion_iniciales,0,0);
    sem_init(&sem_conexion_hecha,0,0);
    sem_init(&sem_fin_memoria,0,0);
}


void inicializar_estructuras(){
    lista_contextos_pids=list_create();
    lista_instrucciones_tid_pid=list_create();
}

void inicializar_mutex(){
    pthread_mutex_init(&mutex_lista_contextos_pids,NULL);
    pthread_mutex_init(&mutex_lista_instruccion,NULL);
}
void destruir_mutex(){
    pthread_mutex_destroy(&mutex_lista_contextos_pids);
    pthread_mutex_destroy(&mutex_lista_instruccion);
}

void destruir_semaforos(){
    sem_destroy(&sem_conexion_iniciales);
    sem_destroy(&sem_conexion_hecha);
    sem_destroy(&sem_fin_memoria);
}

void liberar_instruccion(t_instruccion_tid_pid*instruccion){
    
    free(instruccion->instrucciones->parametros1);

    if(strcmp(instruccion->instrucciones->parametros2,"")==0){
        goto fin;
    }
    else if(strcmp(instruccion->instrucciones->parametros3,"")==0){
        free(instruccion->instrucciones->parametros2);
    }
    else if(strcmp(instruccion->instrucciones->parametros4,"")==0){
        free(instruccion->instrucciones->parametros2);
        free(instruccion->instrucciones->parametros3);
        
    }
    else{
        free(instruccion->instrucciones->parametros2);
        free(instruccion->instrucciones->parametros3);
        free(instruccion->instrucciones->parametros4);
    }
    fin:
    free(instruccion);
}

void eliminar_contexto_pid(t_contexto_pid*contexto_pid){
    for (int i = 0; i<list_size(lista_contextos_pids);i++){
        t_contexto_pid*actual=list_get(lista_contextos_pids,i);
        log_info(logger,"PID CONTEXTO_PID:%d",contexto_pid->pid);
        log_info(logger,"PID CONTEXTO_ACTUAL:%d",actual->pid);
        if (contexto_pid->pid == actual->pid){
            int pid = contexto_pid->pid; // Solo para hacer el log
            list_remove(lista_contextos_pids,i);
            free(contexto_pid->contextos_tids);
            free(contexto_pid);
            log_info(logger,"Contexto del pid %d eliminado",pid);
            return;
        }
    }
}


