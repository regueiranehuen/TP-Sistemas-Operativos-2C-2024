#ifndef PLANIFICACION_H
#define PLANIFICACION_H

#include "commons/collections/queue.h"
#include "funcionesAuxiliares.h"
#include "procesos.h"
#include <semaphore.h>
#include "main.h"

// Hilos planificador largo plazo
extern pthread_t hilo_plani_largo_plazo;
extern pthread_t hilo_new_ready_procesos;
extern pthread_t hilo_exit_procesos;
extern pthread_t hilo_hilos_exit;

// Hilos planificador corto plazo
extern pthread_t hilo_ready_exec;
extern pthread_t hilo_atender_syscall;
extern pthread_t hilo_atender_interrupt;
extern pthread_t hilo_cortar_modulos;

// Hilo para ordenar cola prioridades
extern pthread_t hilo_prioridades;

// Hilo IO
extern pthread_t hilo_IO;
    


t_tcb *fifo_tcb();
void* funcion_new_ready_procesos(void* void_args);
void* funcion_procesos_exit(void* void_args);
void* funcion_hilos_exit(void* void_args);
void* funcion_new_ready_hilos(void* void_args);
void hilo_atender_syscalls();
void planificador_largo_plazo();
void* hilo_planificador_largo_plazo(void* void_args);
void* atender_syscall(void* args);

t_tcb *prioridades();
void round_robin(t_queue*cola);
void colas_multinivel();
int nueva_prioridad(t_list*colas_hilos_prioridad_ready,int priori_actual);
void hilo_ordena_cola_prioridades();
void* ordenamiento_continuo(void* void_args);
void planificador_corto_plazo() ;
void *hilo_planificador_corto_plazo(void *arg);
void *funcion_manejo_procesos(void *arg);

void espera_con_quantum(int quantum);
void ejecucion();
void pushear_cola_ready(t_tcb* hilo);
void* cortar_ejecucion_modulos(void*args);

#endif