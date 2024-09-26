#ifndef PLANIFICACION_H
#define PLANIFICACION_H

#include "commons/collections/queue.h"
#include "funcionesAuxiliares.h"
#include "procesos.h"

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

t_pcb *fifo(t_queue *cola_proceso);
void* funcion_new_ready_procesos(void* void_args);
void* funcion_procesos_exit(void* void_args);
void* funcion_hilos_exit(void* void_args);
void* funcion_new_ready_hilos(void* void_args);
void hilo_atender_syscalls();
void hilo_planificador_largo_plazo();
void* planificador_largo_plazo(void* void_args);
void* atender_syscall(void* void_args);

#endif