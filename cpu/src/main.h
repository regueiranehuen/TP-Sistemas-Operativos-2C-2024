#ifndef MAIN_H
#define MAIN_H

#include "cicloDeInstruccion.c"
#include "funcExecute.c"
#include "mmu.c"
#include "server.c"


void liberarMemoria(t_sockets_cpu * sockets,t_log* log, t_config* config);

#endif