#ifndef MAIN_H
#define MAIN_H

#include "cicloDeInstruccion.h"
#include "funcExecute.h"
#include "mmu.h"
#include "server.h"


void liberarMemoria(t_sockets_cpu * sockets,t_log* log, t_config* config);
void inicializar_estructuras();
void terminar_programa();

#endif