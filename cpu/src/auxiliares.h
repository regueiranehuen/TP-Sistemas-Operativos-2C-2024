#ifndef AUXILIARES_H
#define AUXILIARES_H

#include "server.h"

void inicializar_estructuras();
void inicializar_semaforos();
void inicializar_mutex();
void destruir_mutex();
void select_dispatch();
void terminar_programa();
void destruir_semaforos();

#endif