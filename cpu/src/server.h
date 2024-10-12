#ifndef SERVER_H
#define SERVER_H

#include "utils/sockets.h"

//Variables y structs
typedef struct{
    int socket_Dispatch;
    int socket_Interrupt;
}t_socket;


typedef struct{
    int socket_cliente;
    t_socket* socket_servidor;
}t_sockets_cpu;


t_log* log_cpu;
t_config* config;
t_sockets_cpu* sockets_cpu;
t_contexto* contexto;
t_pcb_exit* pcb_salida;

char* ip_memoria;
int puerto_memoria;
int puerto_escucha_dispatch;
int puerto_escucha_interrupt;
char* log_level;

int socket_servidor_Dispatch, socket_servidor_Interrupt;
int socket_cliente_Dispatch =-1, socket_cliente_Interrupt =-1;
int respuesta_Dispatch, respuesta_Interrupt;
t_socket* sockets;
pthread_t hilo_servidor;
pthread_t hilo_cliente;
void* socket_servidor_kernel;
void* socket_cliente_memoria;
uint32_t tid_interrupt;
int hay_interrupcion;
int es_por_usuario;
int seguir_ejecutando;


//funciones
void leer_config(char* path);
t_socket* servidor_CPU_Kernel(t_log* log, t_config* config);
int cliente_cpu_memoria (t_log* log, t_config * config);
void* funcion_hilo_servidor_cpu(void* void_args);
void* funcion_hilo_cliente_memoria(void* void_args);
t_sockets_cpu* hilos_cpu(t_log* log, t_config* config);

#endif