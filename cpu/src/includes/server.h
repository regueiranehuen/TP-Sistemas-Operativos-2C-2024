#ifndef SERVER_H
#define SERVER_H

#include "utils/includes/sockets.h"

typedef struct{
int socket_Dispatch;
int socket_Interrupt;
}t_socket_cpu;

typedef struct{
t_socket_cpu* socket_cliente;
t_socket_cpu* socket_servidor;
}t_sockets_cpu;

t_socket_cpu* servidor_CPU_Kernel(t_log* log, t_config* config);
t_socket_cpu* cliente_cpu_memoria (t_log* log, t_config * config);
void* función_hilo_servidor_cpu(void* void_args);
void* función_hilo_cliente_memoria(void* void_args);
t_sockets_cpu* hilos_cpu(t_log* log, t_config* config);

#endif