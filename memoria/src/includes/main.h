#ifndef MAIN_H
#define MAIN_H

#include "server.h"
#include "auxiliaresMem.h"

extern int estado_cpu;
extern t_log* logger;
extern t_config* config;

void hilo_recibe_cpu(int socket_servidor_cpu);

#endif