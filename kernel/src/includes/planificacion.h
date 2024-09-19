#ifndef PLANIFICACION_H
#define PLANIFICACION_H

#include "commons/collections/queue.h"
#include "funcionesAuxiliares.h"
#include "procesos.h"


t_pcb *fifo(t_queue *cola_proceso);
t_tcb*round_robin(t_queue*cola_hilos);
t_tcb* prioridades(t_queue* cola_hilos);


#endif