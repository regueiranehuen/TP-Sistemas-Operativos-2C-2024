#ifndef PROCESOS_H
#define PROCESOS_H


#include "commons/collections/list.h"
#include "commons/collections/queue.h"
#include "utils/includes/sockets.h"
#include "../includes/cliente.h"
#include <semaphore.h>

extern sem_t semaforo;
extern t_queue *cola_new;
extern t_queue *cola_ready;
extern t_list *lista_pcbs;
extern pthread_mutex_t mutex_pthread_join;
extern t_config *config;
extern t_log *logger;
extern t_list *lista_mutex;

typedef enum
{
    TCB_NEW,
    TCB_EXECUTE,
    TCB_READY,
    TCB_BLOCKED,
    TCB_BLOCKED_MUTEX,
    TCB_EXIT
} estado_hilo;

typedef struct
{
    int tid;
    int prioridad;
    int pid; // proceso asociado al hilo
    estado_hilo estado;
    char *pseudocodigo;
    int pseudocodigo_length;
} t_tcb;

typedef enum
{
    UNLOCKED,
    LOCKED
} estado_mutex;

typedef enum
{
    PCB_NEW,
    PCB_READY,
    PCB_BLOCKED,
    PCB_EXECUTE,
    PCB_EXIT
} estado_pcb;

typedef struct
{
    int pid;
    t_list *tids;
    t_list *colas_hilos_prioridad_ready;
    t_list *lista_hilos_blocked;
    t_queue *cola_hilos_new;
    t_queue *cola_hilos_exit;
    t_tcb *hilo_exec;

    t_list *lista_mutex; //
    estado_pcb estado; //



    int tamanio_proceso;
    int prioridad;
} t_pcb;

typedef struct
{
    int prioridad;
    t_queue *cola;
} t_cola_prioridad;

typedef struct
{
    t_pcb *pcb;
    t_tcb *tcb;
} t_proceso;

typedef struct
{
    int mutex_id;
    t_queue *cola_tcbs;
    estado_mutex estado;
    t_tcb *hilo; // hilo que esta en la región crítica
} t_mutex;

t_pcb *crear_pcb();
t_tcb *crear_tcb(t_pcb *pcb);

void PROCESS_CREATE(char *pseudocodigo, int tamanio_proceso, int prioridad);
void PROCESS_EXIT();

void THREAD_CREATE(char *pseudocodigo, int prioridad);
void THREAD_JOIN(int tid);
void THREAD_CANCEL(int tid);

void new_a_ready();

void iniciar_kernel(char *archivo_pseudocodigo, int tamanio_proceso);

void MUTEX_CREATE();
void MUTEX_LOCK(int mutex_id);
void MUTEX_UNLOCK(int mutex_id);

void DUMP_MEMORY();
void IO(int milisegundos);

#endif