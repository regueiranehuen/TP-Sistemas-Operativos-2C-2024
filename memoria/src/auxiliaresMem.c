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
    free(instruccion->instrucciones->parametros2);
    free(instruccion->instrucciones->parametros3);
    free(instruccion->instrucciones->parametros4);
    free(instruccion->instrucciones->parametros5);
    free(instruccion->instrucciones->parametros6);

    free(instruccion);
}