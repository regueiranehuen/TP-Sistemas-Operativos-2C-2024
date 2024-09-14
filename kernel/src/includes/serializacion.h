#ifndef SERIALIZACION_H_
#define SERIALIZACION_H_


#include "utils/includes/sockets.h"


void agregar_tcb_a_paquete(t_tcb*tcb,t_paquete*paquete);
void agregar_pcb_a_paquete(t_pcb*pcb,t_paquete*paquete);
void send_tcb(t_tcb*tcb,int socket);
void send_pcb(t_pcb*pcb,int socket);
int tam_tcb(t_tcb * tcb);
int size_tcbs_queue(t_queue*queue);



void enviar_mensaje(char* mensaje, int socket_cliente);
t_paquete* crear_paquete(void);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
void liberar_conexion(int socket_cliente);
void eliminar_paquete(t_paquete* paquete);


#endif