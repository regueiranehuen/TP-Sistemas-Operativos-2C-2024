#ifndef CLIENTE_H
#define CLIENTE_H

#include "utils/includes/sockets.h"
#include "utils/includes/serializacion.h"

typedef struct{
int socket_Dispatch;
int socket_Interrupt;
}t_socket_cpu;

typedef struct{
int socket_cliente_memoria;
t_socket_cpu* sockets_cliente_cpu;
}sockets_kernel;

int cliente_Memoria_Kernel(t_log* log,t_config* config);
t_socket_cpu* cliente_CPU_Kernel(t_log* log, t_config* config);
void* funcion_hilo_cliente_memoria(void* void_args);
void* funcion_hilo_cliente_cpu(void* void_args);
sockets_kernel* hilos_kernel(t_log* log, t_config* config);

#endif