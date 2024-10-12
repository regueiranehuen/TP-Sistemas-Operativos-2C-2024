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

// **Variables Globales**
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

// Sockets y respuestas
int socket_servidor_Dispatch, socket_servidor_Interrupt;
int socket_cliente_Dispatch, socket_cliente_Interrupt;
int respuesta_Dispatch, respuesta_Interrupt;

t_socket_cpu* sockets;  // Estructura de sockets para Dispatch/Interrupt

// Hilos y variables asociadas
pthread_t hilo_servidor;
pthread_t hilo_cliente;
void* socket_servidor_kernel;
void* socket_cliente_memoria;

// Variables de interrupción
uint32_t tid_interrupt;
int hay_interrupcion;
int es_por_usuario;
int seguir_ejecutando;

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
