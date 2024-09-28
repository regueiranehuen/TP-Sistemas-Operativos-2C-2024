#ifndef SERVER_H
#define SERVER_H

#include "utils/sockets.h"
#include "main.h"

typedef struct{
    int socket_Dispatch;
    int socket_Interrupt;
}t_socket_cpu;

typedef struct{
    int socket_cliente;
    t_socket_cpu* socket_servidor;
}t_sockets_cpu;


t_contexto* contexto;
int conexion_memoria;
uint32_t tid_interrupt;
int hay_interrupcion;
int es_por_usuario;
int seguir_ejecutando;
t_socket_cpu* servidor_CPU_Kernel(t_log* log, t_config* config);
int cliente_cpu_memoria (t_log* log, t_config * config);
void* funcion_hilo_servidor_cpu(void* void_args);
void* funcion_hilo_cliente_memoria(void* void_args);
t_sockets_cpu* hilos_cpu(t_log* log, t_config* config);

#endif