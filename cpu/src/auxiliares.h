#ifndef AUXILIARES_H
#define AUXILIARES_H

#include "server.h"

void inicializar_estructuras();
void inicializar_semaforos();
void inicializar_mutex();
void inicializar_mutex_compartido_entre_procesos(pthread_mutex_t* mutex);
void destruir_mutex();
void terminar_programa();
void destruir_semaforos();

#endif