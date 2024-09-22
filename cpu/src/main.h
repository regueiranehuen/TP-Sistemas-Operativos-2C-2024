#ifndef MAIN_H
#define MAIN_H

#include "server.h"

t_log* log_cpu;
t_config* config;
t_sockets_cpu* sockets;
void liberarMemoria(t_sockets_cpu * sockets,t_log* log, t_config* config);

#endif