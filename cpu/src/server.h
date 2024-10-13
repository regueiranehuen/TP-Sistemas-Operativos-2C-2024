#ifndef SERVER_H
#define SERVER_H

#include "utils/sockets.h"
#include <pthread.h>
#include <stdint.h>

// **Estructuras**
typedef struct {
    int socket_Dispatch;
    int socket_Interrupt;
} t_socket_cpu;

typedef struct {
    int socket_cliente;      // Conexión al cliente (Memoria)
    t_socket_cpu* socket_servidor;  // Conexión al Kernel (Dispatch e Interrupt)
} t_sockets_cpu;

extern t_log* log_cpu;
extern t_config* config;
extern t_sockets_cpu* sockets_cpu;
extern t_contexto* contexto;
extern t_pcb_exit* pcb_salida;

extern char* ip_memoria;
extern int puerto_memoria;
extern int puerto_escucha_dispatch;
extern int puerto_escucha_interrupt;
extern char* log_level;

extern int socket_servidor_Dispatch, socket_servidor_Interrupt;
extern int socket_cliente_Dispatch, socket_cliente_Interrupt;
extern int respuesta_Dispatch, respuesta_Interrupt;

extern t_socket_cpu* sockets;

extern pthread_t hilo_servidor;
extern pthread_t hilo_cliente;
extern void* socket_servidor_kernel;
extern void* socket_cliente_memoria;

extern uint32_t tid_interrupt;
extern int hay_interrupcion;
extern int es_por_usuario;


// **Funciones**
void leer_config(char* path);
t_socket_cpu* servidor_CPU_Kernel(t_log* log, t_config* config);
int cliente_cpu_memoria(t_log* log, t_config* config);

void* funcion_hilo_servidor_cpu(void* void_args);
void* funcion_hilo_cliente_memoria(void* void_args);

t_sockets_cpu* hilos_cpu(t_log* log, t_config* config);

void recibir_kernel_dispatch(int socket_cliente_Dispatch);
void recibir_kernel_interrupt(int socket_cliente_Interrupt);
void ejecutar_ciclo_de_instruccion(t_log* log);

#endif  // SERVER_H
