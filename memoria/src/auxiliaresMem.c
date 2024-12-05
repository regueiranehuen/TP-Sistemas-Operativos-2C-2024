#include "includes/auxiliaresMem.h"

void inicializar_semaforos(){
    sem_init(&sem_conexion_iniciales,0,0);
    sem_init(&sem_conexion_hecha,0,0);
    sem_init(&sem_fin_memoria,0,0);
    sem_init(&sem_termina_hilo,0,0);
}

void eliminar_estructuras()
{
    pthread_mutex_lock(&mutex_lista_contextos_pids);
    if (!list_is_empty(lista_contextos_pids))
    {
        for (int i = 0; i < list_size(lista_contextos_pids); i++)
        {
            t_contexto_pid *contexto_pid = list_get(lista_contextos_pids, i);
            list_remove_element(lista_contextos_pids, contexto_pid);
            if (!list_is_empty(contexto_pid->contextos_tids))
            {
                for (int j = 0; j < list_size(contexto_pid->contextos_tids); j++)
                {
                    t_contexto_tid *contexto_tid = list_get(contexto_pid->contextos_tids, j);
                    list_remove_element(contexto_pid->contextos_tids, contexto_tid);
                    free(contexto_tid->registros);
                    free(contexto_tid);
                    j--;
                }
            }

            list_destroy(contexto_pid->contextos_tids);
            free(contexto_pid);
            i--;
        }
    }

    list_destroy(lista_contextos_pids);
    pthread_mutex_unlock(&mutex_lista_contextos_pids);

    pthread_mutex_lock(&mutex_lista_instruccion);

    if (!list_is_empty(lista_instrucciones_tid_pid))
    {
        for (int i = 0; i < list_size(lista_instrucciones_tid_pid); i++)
        {
            t_instruccion_tid_pid *actual = list_get(lista_instrucciones_tid_pid, i);

            list_remove(lista_instrucciones_tid_pid, i);
            liberar_instruccion(actual);

            i--; // Decrementa i para no saltar el siguiente elemento
        }
    }

    list_destroy_and_destroy_elements(lista_particiones,free);

    list_destroy(lista_instrucciones_tid_pid);
    pthread_mutex_unlock(&mutex_lista_instruccion);
}

void inicializar_estructuras(){
    lista_contextos_pids=list_create();
    lista_instrucciones_tid_pid=list_create();
}

void inicializar_mutex(){
    pthread_mutex_init(&mutex_lista_contextos_pids,NULL);
    pthread_mutex_init(&mutex_lista_instruccion,NULL);
    pthread_mutex_init(&mutex_estado_memoria,NULL);
    pthread_mutex_init(&mutex_lista_particiones,NULL);
    pthread_mutex_init(&mutex_logs,NULL);
}

void destruir_mutex(){
    pthread_mutex_destroy(&mutex_lista_contextos_pids);
    pthread_mutex_destroy(&mutex_lista_instruccion);
    pthread_mutex_destroy(&mutex_estado_memoria);
    pthread_mutex_destroy(&mutex_lista_particiones);
    pthread_mutex_destroy(&mutex_logs);
}

void destruir_semaforos(){
    sem_destroy(&sem_conexion_iniciales);
    sem_destroy(&sem_conexion_hecha);
    sem_destroy(&sem_fin_memoria);
    sem_destroy(&sem_termina_hilo);
}

void liberar_instruccion(t_instruccion_tid_pid* instruccion) {
    
    free(instruccion->instrucciones->parametros1);

    if (strcmp(instruccion->instrucciones->parametros2, "") != 0) {
        free(instruccion->instrucciones->parametros2);
    }

    if (strcmp(instruccion->instrucciones->parametros3, "") != 0) {
        free(instruccion->instrucciones->parametros3);
    }

    if (strcmp(instruccion->instrucciones->parametros4, "") != 0) {
        free(instruccion->instrucciones->parametros4);
    }

    free(instruccion->instrucciones);
    free(instruccion);
}

void eliminar_contexto_pid(t_contexto_pid*contexto_pid){
    for (int i = 0; i<list_size(lista_contextos_pids);i++){
        t_contexto_pid*actual=list_get(lista_contextos_pids,i);
        pthread_mutex_lock(&mutex_logs);
        log_info(logger,"PID CONTEXTO_PID:%d",contexto_pid->pid);
        log_info(logger,"PID CONTEXTO_ACTUAL:%d",actual->pid);
        pthread_mutex_unlock(&mutex_logs);
        if (contexto_pid->pid == actual->pid){
            int pid = contexto_pid->pid; // Solo para hacer el log

            for (int j = 0; j<list_size(actual->contextos_tids);j++){
                t_contexto_tid*act = list_get(actual->contextos_tids,j);
                pthread_mutex_lock(&mutex_logs);
                log_info(logger,"TID CONTEXTO_ACTUAL:%d",act->tid);
                log_info(logger,"AX: %d",act->registros->AX);
                pthread_mutex_unlock(&mutex_logs);
                list_remove(actual->contextos_tids,j);
                free(act->registros);
                free(act);
                j--; // Acomodar la lista de contextos tids
            }
            list_destroy(actual->contextos_tids);
            list_remove(lista_contextos_pids,i);
            free(contexto_pid);
            pthread_mutex_lock(&mutex_logs);
            log_info(logger,"Contexto del pid %d eliminado",pid);
            pthread_mutex_unlock(&mutex_logs);
            return;
        }
    }
}


void print_pids(t_list* lista_contextos) {
    // Recorremos la lista de contextos
    for (int i = 0; i < list_size(lista_contextos); i++) {
        // Obtenemos el contexto en la posición i
        t_contexto_pid* contexto = list_get(lista_contextos, i);
        // Imprimimos el pid
        pthread_mutex_lock(&mutex_logs);
        log_info(logger,"PID: %d", contexto->pid);
        pthread_mutex_unlock(&mutex_logs);
    }
}