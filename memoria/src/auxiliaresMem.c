#include "includes/auxiliaresMem.h"

void inicializar_semaforos(){
    sem_init(&sem_conexion_hecha,0,0);
}


void inicializar_mutex(){
    pthread_mutex_init(&mutex_lista_contextos_pids,NULL);
}
void destruir_mutex(){
    pthread_mutex_destroy(&mutex_lista_contextos_pids);
}

void destruir_semaforos(){
    sem_destroy(&sem_conexion_hecha);
}
