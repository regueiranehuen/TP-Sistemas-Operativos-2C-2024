#ifndef SERVER_H
#define SERVER_H

#include "utils/includes/sockets.h"
#include "utils/includes/estructuras.h"
#include <pthread.h>
#include <stdint.h>

// **Estructuras**
typedef struct {
    int socket_servidor_Dispatch;
    int socket_servidor_Interrupt;
    int socket_cliente_Dispatch;
    int socket_cliente_Interrupt;
} t_socket_cpu;

typedef struct {
    int socket_memoria;      // Conexión al cliente (Memoria). NOMBRE CAMBIADO: DE socket_cliente A socket_memoria
    t_socket_cpu* socket_servidor;  // Conexión al Kernel (Dispatch e Interrupt)
} t_sockets_cpu;

extern t_log* log_cpu;
extern t_config* config;
extern t_sockets_cpu* sockets_cpu;

extern char* ip_memoria;
extern int puerto_memoria;
extern int puerto_escucha_dispatch;
extern int puerto_escucha_interrupt;
extern char* log_level_config;

extern int socket_servidor_Dispatch, socket_servidor_Interrupt;
extern int socket_cliente_Dispatch, socket_cliente_Interrupt;
extern int respuesta_Dispatch, respuesta_Interrupt;

extern t_socket_cpu* sockets;

extern pthread_t hilo_servidor;
extern pthread_t hilo_cliente;
extern void* socket_servidor_kernel;
extern void* socket_cliente_memoria;

extern uint32_t tid_interrupt;
extern bool hay_interrupcion;
extern bool seguir_ejecutando;

extern t_contexto_tid*contexto_tid_actual;
extern t_contexto_pid*contexto_pid_actual;

extern pthread_mutex_t mutex_contextos_exec;
extern pthread_mutex_t mutex_interrupt;
extern pthread_mutex_t mutex_motivo_devolucion;

extern sem_t sem_ciclo_instruccion;
extern sem_t sem_ok_o_interrupcion;
extern sem_t sem_finalizacion_cpu;

extern code_operacion devolucion_kernel;

// **Funciones**
void leer_config(char* path);
t_socket_cpu* servidor_CPU_Kernel(t_log* log, t_config* config);
int cliente_cpu_memoria(t_log* log, t_config* config);

void* funcion_hilo_servidor_cpu(void* void_args);
void* funcion_hilo_cliente_memoria(void* void_args);

t_sockets_cpu* hilos_cpu(t_log* log, t_config* config);



void ejecutar_ciclo_de_instruccion(t_log* log);
bool es_interrupcion(syscalls code);
void inicializar_semaforos();
void destruir_semaforos();
void* recibir_kernel_dispatch(void*args);
void* recibir_kernel_interrupt(void*args);


#endif  // SERVER_H
