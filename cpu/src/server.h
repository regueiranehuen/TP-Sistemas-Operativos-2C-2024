#ifndef SERVER_H
#define SERVER_H

#include "utils/includes/sockets.h"
#include "utils/includes/estructuras.h"
#include <pthread.h>
#include <stdint.h>

// **Estructuras**
typedef struct {
    int socket_Dispatch;
    int socket_Interrupt;
} t_socket_cpu;

typedef struct {
    int socket_memoria;      // Conexión al cliente (Memoria). NOMBRE CAMBIADO: DE socket_cliente A socket_memoria
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
t_config* iniciar_config(char *path);
void liberar_config(t_config* config);

t_socket_cpu* servidor_CPU_Kernel(t_log* log, t_config* config);
int cliente_cpu_memoria(t_log* log, t_config* config);

void* funcion_hilo_servidor_cpu(void* void_args);
void* funcion_hilo_cliente_memoria(void* void_args);

t_sockets_cpu* hilos_cpu(t_log* log, t_config* config);

void recibir_kernel_dispatch(int socket_cliente_Dispatch);
void recibir_kernel_interrupt(int socket_cliente_Interrupt);
void ejecutar_ciclo_de_instruccion(t_log* log);
bool es_interrupcion_usuario(code_operacion code);

t_contexto* crear_contexto(int tid, uint32_t base, uint32_t limite);
int existe_contexto(t_contexto_pid* contexto_pid, int tid);
int agregar_contexto_tid(t_contexto_pid* contexto_pid, int tid);
t_registros_cpu* inicializar_registros_cpu();
t_contexto* inicializar_contexto(int tid);
t_contexto_pid* inicializar_contexto_pid(int pid);
t_contexto_tid* inicializar_contexto_tid(int tid);
t_pcb* inicializar_pcb(t_contexto* contexto);
void liberar_registros_cpu(t_registros_cpu* registros);
void liberar_contexto(t_contexto* contexto);
void liberar_contexto_pid(t_contexto_pid* contexto_pid);
void liberar_contexto_tid(t_contexto_tid* contexto_tid);
void liberar_pcb(t_pcb* pcb);





#endif  // SERVER_H
