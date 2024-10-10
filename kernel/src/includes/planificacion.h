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
ENUM_DUMP_MEMORY,
ENUM_SEGMENTATION_FAULT
}syscalls;

extern int estado_kernel;
extern sem_t sem_desalojado;

t_tcb *fifo_tcb();
void* funcion_new_ready_procesos(void* void_args);
void* funcion_procesos_exit(void* void_args);
void* funcion_hilos_exit(void* void_args);
void* funcion_new_ready_hilos(void* void_args);
void hilo_atender_syscalls();
void hilo_planificador_largo_plazo();
void* planificador_largo_plazo(void* void_args);
void atender_syscall();

t_tcb *prioridades();
void round_robin(t_queue*cola);
void colas_multinivel();
int nueva_prioridad(t_list*colas_hilos_prioridad_ready,int priori_actual);
void hilo_ordena_cola_prioridades();
void* ordenamiento_continuo(void* void_args);
void planificador_corto_plazo() ;
void *hilo_planificador_corto_plazo(void *arg);
void *funcion_manejo_procesos(void *arg);
#endif