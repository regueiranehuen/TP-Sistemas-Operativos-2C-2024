#ifndef SERIALIZACION_H
#define SERIALIZACION_H

#include "utils/includes/sockets.h"

#include "../../kernel/src/includes/planificacion.h"

typedef enum
{
	MENSAJE,
	PAQUETE
}op_code;

typedef struct {
    int size; // Tamaño del payload
    void* stream; // Payload (todo lo que viene después del codigo de operacion)
} t_buffer;



// typedef struct
// {
// 	int size;
// 	void* stream;
// } t_buffer;

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
	op_code codigo_operacion;
	t_buffer* buffer;
}t_paquete;

void agregar_tcb_a_paquete(t_tcb*tcb,t_paquete*paquete);
void agregar_pcb_a_paquete(t_pcb*pcb,t_paquete*paquete);
void send_tcb(t_tcb*tcb,int socket);
void send_pcb(t_pcb*pcb,int socket);
void send_tid(t_tcb*tcb,int socket_cliente);
void send_pid(t_pcb*pcb,int socket_cliente);

void enviar_mensaje(char* mensaje, int socket_cliente);
t_paquete* crear_paquete(void);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
void liberar_conexion(int socket_cliente);
void eliminar_paquete(t_paquete* paquete);

t_list* recibir_paquete(int socket_cliente);
void* recibir_buffer(int* size, int socket_cliente);




void send_operacion_entero(code_operacion code, int entero, int socket_cliente);
void send_operacion_tid_pid(code_operacion code, int tid, int pid, int socket_cliente);
void send_paquete_code_operacion(code_operacion code, t_buffer*buffer, int socket_cliente);
void send_paquete_syscall(t_buffer*buffer);
t_paquete_syscall* recibir_paquete_syscall(void);
t_list* parametros_process_create(t_buffer*buffer);
t_list* parametros_thread_create(t_buffer*buffer);
int recibir_entero_buffer(t_buffer*buffer);
char* recibir_string_buffer(t_buffer*buffer);

#endif