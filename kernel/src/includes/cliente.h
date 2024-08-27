#ifndef CLIENTE_H
#define CLIENTE_H

#include "utils/includes/sockets.h"

typedef struct{
int socket_Dispatch;
int socket_Interrupt;
}t_socket_cpu;

int cliente_Memoria_Kernel(t_log* log,t_config* config);
t_socket_cpu cliente_CPU_Kernel(t_log* log, t_config* config);


#endif