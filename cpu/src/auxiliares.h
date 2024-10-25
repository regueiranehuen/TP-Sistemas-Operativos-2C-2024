#ifndef AUXILIARES_H
#define AUXILIARES_H

#include "utils/includes/semaforosCompartidos.h"
#include "server.h"

void inicializar_estructuras();
void inicializar_semaforos();
void inicializar_mutex();
void inicializar_mutex_compartido_entre_procesos(pthread_mutex_t* mutex);
void destruir_mutex();
void terminar_programa();
void liberarMemoria(t_sockets_cpu * sockets,t_log* log, t_config* config);
void destruir_semaforos();

#endif