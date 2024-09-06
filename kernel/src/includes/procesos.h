#include "commons/collections/list.h"
#include "commons/collections/queue.h"
#include "utils/includes/sockets.h"
#include "includes/cliente.h"
#include <semaphore.h>

extern sem_t semaforo;

typedef struct{
int tid;
int prioridad;
char* estado;
}t_tcb;

typedef struct{
int pid;
t_list* tids;
char* mutex;
char* estado;
}t_pcb;

typedef struct{
t_pcb* pcb;
t_tcb* tcb;
}t_proceso;

t_pcb* crear_pcb();
t_tcb *crear_tcb(t_pcb *pcb);
void crear_proceso(t_queue *cola_new_procesos, int socket_memoria, t_queue *cola_ready_proceso);
void new_a_ready(t_queue *cola_new_procesos, int socket_memoria, t_queue *cola_ready_proceso);
t_proceso iniciar_kernel (char* archivo_pseudocodigo, int tamanio_proceso);
void finalizar_proceso(t_pcb *pcb, int socket_memoria);
void liberar_pcb(t_pcb *pcb);
