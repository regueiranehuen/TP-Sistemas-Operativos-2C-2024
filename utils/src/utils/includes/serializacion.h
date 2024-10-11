#ifndef SERIALIZACION_H
#define SERIALIZACION_H

#include "sockets.h"

typedef enum{
    DUMP_MEMORIA,
    PROCESS_EXIT_AVISO, 
    PROCESS_CREATE_AVISO,
    THREAD_CREATE_AVISO,
    THREAD_ELIMINATE_AVISO,
    THREAD_EXECUTE_AVISO,
    THREAD_CANCEL_AVISO,
    THREAD_INTERRUPT,
    FIN_QUANTUM_RR,
}code_operacion;

typedef enum{
ENUM_PROCESS_CREATE,
ENUM_PROCESS_EXIT,
ENUM_THREAD_CREATE,
ENUM_THREAD_JOIN,
ENUM_THREAD_CANCEL,
ENUM_MUTEX_CREATE,
ENUM_MUTEX_LOCK,
ENUM_MUTEX_UNLOCK,
ENUM_IO,
ENUM_DUMP_MEMORY,
ENUM_SEGMENTATION_FAULT
}syscalls;

typedef struct {
    int size; // Tamaño del payload
    void* stream; // Payload (todo lo que viene después del codigo de operacion)
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
	char*nombreArchivo;
	int tamProceso;
	int prioridad;
}t_process_create;

typedef struct{
	char*nombreArchivo;
	int prioridad;
}t_thread_create;




t_process_create* parametros_process_create(t_paquete_syscall*paquete);
t_thread_create* parametros_thread_create(t_paquete_syscall*paquete);
int recibir_entero_paquete_syscall(t_paquete_syscall*paquete);
char* recibir_string_paquete_syscall(t_paquete_syscall*paquete);
void send_operacion_tid_pid(code_operacion code, int tid, int pid, int socket_cliente);
void send_operacion_entero(code_operacion code, int entero, int socket_cliente);
void send_operacion_pid(code_operacion code, int pid, int socket_cliente);
void send_paquete_syscall(t_buffer*buffer, int socket_cliente);
void send_paquete_code_operacion(code_operacion code, t_buffer*buffer, int socket_cliente);
t_paquete_syscall* recibir_paquete_syscall(int socket_dispatch);
t_list* recibir_paquete_code_operacion(int socket_cliente);
void liberar_conexion(int socket_cliente);

void eliminar_paquete_syscall(t_paquete_syscall*paquete);
void eliminar_paquete_code_op(t_paquete_code_operacion*paquete);

#endif