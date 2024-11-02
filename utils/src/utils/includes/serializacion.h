#ifndef SERIALIZACION_H
#define SERIALIZACION_H

#include "sockets.h"



typedef enum{
    TERMINAR=-1, /////
    INICIALIZAR_PROCESO,
    DUMP_MEMORIA,
    PROCESS_EXIT_AVISO, 
    PROCESS_CREATE_AVISO,
    THREAD_CREATE_AVISO,
    THREAD_ELIMINATE_AVISO,
    THREAD_EXECUTE_AVISO,
    THREAD_CANCEL_AVISO,
    THREAD_INTERRUPT,
    FIN_QUANTUM_RR,
    DESALOJAR,
    SEGMENTATION_FAULT,
    OK,
    KERNEL,
    CPU
}code_operacion;

typedef enum{
ENUM_PROCESS_CREATE,
ENUM_PROCESS_EXIT,
ENUM_THREAD_CREATE,
ENUM_THREAD_JOIN,
ENUM_THREAD_CANCEL,
ENUM_THREAD_EXIT, // Faltaba este
ENUM_MUTEX_CREATE,
ENUM_MUTEX_LOCK,
ENUM_MUTEX_UNLOCK,
ENUM_IO,
ENUM_DUMP_MEMORY,
ENUM_SEGMENTATION_FAULT,
ENUM_FIN_QUANTUM_RR,
ENUM_DESALOJAR
}syscalls;

typedef struct {
    uint32_t size; // Tamaño del payload
    uint32_t offset; // Desplazamiento dentro del payload
    void* stream; // Payload
} t_buffer;

typedef struct{
	code_operacion code; // Header
	t_buffer*buffer;
}t_paquete_code_operacion;

typedef struct
{
	syscalls syscall; // Header
	t_buffer* buffer;
} t_paquete_syscall;

typedef struct{
    int tid;
    int pid;
}t_tid_pid;


typedef struct{
	char*nombreArchivo;
	int tamProceso;
	int prioridad;
}t_process_create;

typedef struct{
	char*nombreArchivo;
	int prioridad;
}t_thread_create;

typedef struct{
    int tamanio_proceso;
    int pid;
}t_process_create_mem;


typedef struct{
    int pid;
    int tam_proceso;
    char*arch_pseudocodigo;
}t_args_inicializar_proceso;

typedef struct{
    int pid;
    int tid;
    char*arch_pseudo;
}t_args_thread_create_aviso;


void send_process_create(char* nombreArchivo, int tamProceso, int prioridad, int socket_cliente);
void send_thread_create(char*nombreArchivo,int prioridad,int socket_cliente);
void send_process_exit(int socket_cliente);
void send_thread_join(int tid, int socket_cliente);
void send_thread_cancel(int tid, int socket_cliente);
void send_mutex_create(char* recurso, int socket_cliente);
void send_mutex_lock(char* recurso, int socket_cliente);
void send_mutex_unlock(char* recurso, int socket_cliente);
void send_IO(int milisegundos, int socket_cliente);
void send_dump_memory(int socket_cliente);

void send_paquete_syscall_sin_parametros(int socket_cliente, syscalls syscall);
void send_paquete_syscall(t_buffer*buffer, int socket_cliente,syscalls syscall);
t_paquete_syscall* recibir_paquete_syscall(int socket_dispatch);

int recibir_entero_buffer(t_paquete_syscall*paquete);

t_process_create* parametros_process_create(t_paquete_syscall*paquete);
t_thread_create* parametros_thread_create(t_paquete_syscall*paquete);
int recibir_entero_paquete_syscall(t_paquete_syscall*paquete);
char* recibir_string_paquete_syscall(t_paquete_syscall*paquete);
void send_operacion_tid_pid(code_operacion code, int tid, int pid, int socket_cliente);
void send_operacion_entero(code_operacion code, int entero, int socket_cliente);
void send_operacion_pid(code_operacion code, int pid, int socket_cliente);
void send_paquete_syscall(t_buffer*buffer, int socket_cliente, syscalls syscall);
void send_paquete_code_operacion(code_operacion code, t_buffer*buffer, int socket_cliente);
t_paquete_syscall* recibir_paquete_syscall(int socket_dispatch);
void liberar_conexion(int socket_cliente);
void send_paquete_solo_code_operacion(int socket_cliente,code_operacion code,t_paquete_code_operacion*paquete);
t_paquete_code_operacion* recibir_paquete_code_operacion(int socket_cliente);
t_tid_pid* recepcionar_tid_pid_code_op(t_paquete_code_operacion* paquete);
int recepcionar_int_code_op(t_paquete_code_operacion* paquete);
t_process_create_mem* recepcionar_pid_tamanio(t_paquete_code_operacion* paquete);
void send_operacion_pid_tamanio_proceso(code_operacion code, int pid, int tamanio_proceso, int socket_cliente);

void eliminar_paquete_syscall(t_paquete_syscall*paquete);
void eliminar_paquete_code_op(t_paquete_code_operacion*paquete);
void send_thread_exit(int socket_cliente);

code_operacion recibir_code_operacion(int socket_cliente);
void send_code_operacion(code_operacion code, int socket_cliente);
void send_operacion_tid(code_operacion code, int tid, int socket_cliente);

void send_inicializacion_proceso(int pid, char*arch_pseudocodigo,int tamanio_proceso, int socket_cliente);
t_args_inicializar_proceso* recepcionar_inicializacion_proceso(t_paquete_code_operacion*paquete);

void send_fin_quantum_rr(int socket_cliente);
void send_desalojo(int socket_cliente);
void send_segmentation_fault(int socket_cliente);
void send_inicializacion_hilo(int tid, int pid, char*arch_pseudocodigo,int socket_cliente);
t_args_thread_create_aviso* recepcionar_inicializacion_hilo(t_paquete_code_operacion*paquete);
char* obtener_ruta_absoluta(const char *ruta_relativa);

t_log_level log_level(t_config* config);

#endif