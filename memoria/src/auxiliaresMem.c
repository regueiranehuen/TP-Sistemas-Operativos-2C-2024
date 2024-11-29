#include "includes/auxiliaresMem.h"

void inicializar_semaforos(){
    sem_init(&sem_conexion_iniciales,0,0);
    sem_init(&sem_conexion_hecha,0,0);
    sem_init(&sem_fin_memoria,0,0);
    sem_init(&sem_termina_hilo,0,0);
}

void eliminar_estructuras(){
    pthread_mutex_lock(&mutex_lista_contextos_pids);
    for (int i = 0; i < list_size(lista_contextos_pids); i++){
        t_contexto_pid*contexto_pid = list_get(lista_contextos_pids,i);
        list_remove_element(lista_contextos_pids,contexto_pid);
        for (int j = 0; j < list_size(contexto_pid->contextos_tids); j++){
            t_contexto_tid*contexto_tid = list_get(contexto_pid->contextos_tids,j);
            list_remove_element(contexto_pid->contextos_tids,contexto_tid);
            free(contexto_tid->registros);
            free(contexto_tid);
            j--;
        }
        free(contexto_pid->contextos_tids);
        free(contexto_pid);
        i--;
    }
    free(lista_contextos_pids);
    pthread_mutex_unlock(&mutex_lista_contextos_pids);

    pthread_mutex_lock(&mutex_lista_instruccion);
    for (int i = 0; i < list_size(lista_instrucciones_tid_pid); i++){
        t_instruccion_tid_pid *actual = list_get(lista_instrucciones_tid_pid, i);

        list_remove(lista_instrucciones_tid_pid, i);
        liberar_instruccion(actual);

        i--; // Decrementa i para no saltar el siguiente elemento
    }
    free(lista_instrucciones_tid_pid);
    pthread_mutex_unlock(&mutex_lista_instruccion);
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
    sem_destroy(&sem_termina_hilo);
}

void liberar_instruccion(t_instruccion_tid_pid* instruccion) {
    // Liberar siempre el nombre de la instrucción
    /*free(instruccion->instrucciones->parametros1);
    free(instruccion->instrucciones->parametros2);
    free(instruccion->instrucciones->parametros3);
    free(instruccion->instrucciones->parametros4);
    free(instruccion);*/

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

void detectar_cierres(int socket_cpu,int socket_kernel){
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(socket_cpu,&readfds);

    int actividad = select(socket_cpu + 1,&readfds,NULL,NULL,NULL);

    if (actividad < 0)
    {
        log_error(logger,"Error en select (socket conexion cpu)");
        exit(EXIT_FAILURE);
    }

    // Si se detecta actividad en el socket de conexion con cpu
    if (FD_ISSET(socket_cpu, &readfds))
    {
        log_info(logger,"Se detectó actividad en el socket de conexion con cpu");
    }

    fd_set readfds2;

    FD_ZERO(&readfds2);
    FD_SET(socket_kernel,&readfds2);

    actividad = select(socket_kernel + 1,&readfds2,NULL,NULL,NULL);

    if (actividad < 0)
    {
        log_error(logger,"Error en select (socket conexion kernel)");
        exit(EXIT_FAILURE);
    }

    // Si se detecta actividad en el socket de conexion con cpu
    if (FD_ISSET(socket_kernel, &readfds2))
    {
        log_info(logger,"Se detectó actividad en el socket de conexion con kernel");
    }
}
