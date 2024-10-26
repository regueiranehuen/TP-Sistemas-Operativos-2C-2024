#include "includes/auxiliaresMem.h"

void inicializar_semaforos(){
    sem_init(&sem_conexion_hecha,0,0);
    sem_init(&sem_fin_memoria,0,0);
}


void inicializar_estructuras(){
    lista_contextos_pids=list_create();
    lista_instrucciones_tid_pid=list_create();
}

void inicializar_mutex(){
    pthread_mutex_init(&mutex_lista_contextos_pids,NULL);
}
void destruir_mutex(){
    pthread_mutex_destroy(&mutex_lista_contextos_pids);
}

void destruir_semaforos(){
    sem_destroy(&sem_conexion_hecha);
    sem_destroy(&sem_fin_memoria);
}
