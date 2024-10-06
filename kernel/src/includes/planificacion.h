#ifndef PLANIFICACION_H
#define PLANIFICACION_H

#include "commons/collections/queue.h"
#include "funcionesAuxiliares.h"
#include "procesos.h"
#include <semaphore.h>

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
ENUM_DUMP_MEMORY
}syscalls;

extern int estado_kernel;

t_pcb *fifo_pcb(t_queue *cola_proceso);
t_tcb *fifo_tcb(t_pcb* pcb);
void* funcion_new_ready_procesos(void* void_args);
void* funcion_procesos_exit(void* void_args);
void* funcion_hilos_exit(void* void_args);
void* funcion_new_ready_hilos(void* void_args);
void hilo_atender_syscalls();
void hilo_planificador_largo_plazo();
void* planificador_largo_plazo(void* void_args);
void* atender_syscall(void* void_args);
void planificador_corto_plazo(t_pcb*pcb);

t_tcb *fifo_tcb(t_pcb* pcb);
t_tcb *prioridades(t_pcb *pcb);
void round_robin(t_queue*cola);
void colas_multinivel(t_pcb *pcb);
int nueva_prioridad(t_list*colas_hilos_prioridad_ready,int priori_actual);
void hilo_ordena_cola_prioridades(t_pcb* pcb);
void* ordenamiento_continuo(void* void_args);
void planificador_corto_plazo(t_pcb*pcb) ;
void *funcion_ready_exec_hilos(void *arg);
void *funcion_manejo_procesos(void *arg);
#endif