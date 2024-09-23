#ifndef PLANIFICACION_H
#define PLANIFICACION_H

#include "commons/collections/queue.h"
#include "funcionesAuxiliares.h"
#include "procesos.h"

#include <semaphore.h>



t_pcb *fifo(t_queue *cola_proceso);
t_tcb* prioridades(t_queue* cola_hilos);
t_tcb*round_robin(t_queue*cola);
t_tcb* colas_multinivel(t_pcb*pcb,int*prioridad); 


#endif