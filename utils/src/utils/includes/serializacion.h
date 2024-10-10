#ifndef SERIALIZACION_H_
#define SERIALIZACION_H_

#include "utils/includes/sockets.h"
#include "../../kernel/src/includes/funcionesAuxiliares.h"

typedef enum
{
	MENSAJE,
	PAQUETE
}op_code;

typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

void agregar_tid_a_paquete(t_tcb* tcb, t_paquete* paquete);
void agregar_pid_a_paquete(t_pcb* pcb, t_paquete* paquete);
void send_tid(t_tcb* tcb, int socket_memoria);
void send_pid(t_pcb* pcb, int socket_memoria);

void enviar_mensaje(char* mensaje, int socket_cliente);
t_paquete* crear_paquete(void);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
void liberar_conexion(int socket_cliente);
void eliminar_paquete(t_paquete* paquete);
void* serializar_paquete(t_paquete* paquete, int bytes);
void crear_buffer(t_paquete* paquete);

#endif