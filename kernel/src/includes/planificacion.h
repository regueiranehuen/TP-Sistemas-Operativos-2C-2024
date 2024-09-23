#ifndef PLANIFICACION_H
#define PLANIFICACION_H

#include "commons/collections/queue.h"
#include "funcionesAuxiliares.h"
#include "procesos.h"

#include <semaphore.h>




t_pcb *fifo_pcb(t_queue *cola_proceso);

t_tcb *fifo_tcb(t_pcb* pcb);
t_tcb *prioridades(t_pcb *pcb);
t_tcb*round_robin(t_queue*cola);
t_tcb* colas_multinivel(t_pcb*pcb,int prioridad); 
int nueva_prioridad(t_list*colas_hilos_prioridad_ready,int priori_actual);


#endif