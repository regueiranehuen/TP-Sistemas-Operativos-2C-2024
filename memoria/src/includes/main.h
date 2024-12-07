#ifndef MAIN_H
#define MAIN_H

#include "server.h"
#include "auxiliaresMem.h"

extern int estado_memoria;
extern t_log* logger;
extern t_config* config;

void hilo_recibe_cpu();

#endif