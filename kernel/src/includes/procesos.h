#ifndef PROCESOS_H
#define PROCESOS_H
#include "commons/collections/list.h"
#include "commons/collections/queue.h"
#include "utils/includes/sockets.h"
#include "includes/cliente.h"
#include <semaphore.h>

extern sem_t semaforo;
extern t_queue *cola_new;
extern t_queue *cola_ready;
extern t_list *lista_pcbs;
extern pthread_mutex_t mutex_pthread_join;
typedef struct
{
    int tid;
    int prioridad;
    int pid; // proceso asociado al hilo



    char *estado;
    char *pseudocodigo;

    int estado_length;
    int pseudocodigo_length;

    // No se si me conviene hacer el void * stream acá o trabajarlo x afuera, por ahora lo hago x afuera

} t_tcb;

typedef struct
{
    int pid;
    t_list *tids;
    t_list *colas_hilos_prioridad_ready;
    t_list *lista_hilos_blocked;
    t_queue *cola_hilos_new;
    t_queue *cola_hilos_exit;
    t_tcb *hilo_exec;
    char *mutex;
    char *estado;
    char *pseudocodigo;
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

t_pcb *crear_pcb();
t_tcb *crear_tcb(t_pcb *pcb);

t_pcb *PROCESS_CREATE(char *pseudocodigo, int tamanio_proceso, int prioridad);
void PROCESS_EXIT(t_log *log, t_config *config);

t_tcb* THREAD_CREATE (char* pseudocodigo,int prioridad,t_log* log, t_config* config);
void THREAD_JOIN(int tid);
void THREAD_CANCEL(int tid, t_config *config, t_log *log);

void new_a_ready(int socket_memoria);

void iniciar_kernel(char *archivo_pseudocodigo, int tamanio_proceso);



#endif