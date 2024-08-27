#ifndef SERVER_H
#define SERVER_H

#include "utils/includes/sockets.h"

typedef struct{
int socket_Dispatch;
int socket_Interrupt;
}t_socket_cpu;

t_socket_cpu servidor_CPU_Kernel(t_log* log, t_config* config);

#endif