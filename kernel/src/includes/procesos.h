#include "commons/collections/list.h"
#include "commons/collections/queue.h"
#include "utils/includes/sockets.h"
#include "includes/cliente.h"
#include <semaphore.h>

extern sem_t semaforo;
extern t_queue* cola_new;
extern t_queue* cola_ready;
extern t_list* lista_pcbs;
typedef struct{
int tid;
int prioridad;
int pid; // proceso asociado al hilo
char* estado;
char* pseudocodigo;
}t_tcb;

typedef struct{
int pid;
t_list* tids;
t_list* colas_hilos_prioridad_ready;
t_list* lista_hilos_blocked;
t_queue* cola_hilos_new;
t_queue* cola_hilos_exit;
t_tcb* hilo_exec;
char* mutex;
char* estado;
char* pseudocodigo;
int tamanio_proceso;
}t_pcb;

typedef struct {
    int prioridad;
    t_queue* cola;
} t_cola_prioridad;

typedef struct{
t_pcb* pcb;
t_tcb* tcb;
}t_proceso;

t_pcb* crear_pcb();
t_tcb *crear_tcb(t_pcb *pcb);
t_pcb* PROCESS_CREATE (char* pseudocodigo,int tamanio_proceso,int prioridad);
void new_a_ready(int socket_memoria);
void PROCESS_EXIT(t_tcb* tcb,int socket_memoria);
t_pcb* lista_pcb(t_list* lista_pcbs, int pid);
t_proceso iniciar_kernel (char* archivo_pseudocodigo, int tamanio_proceso);
void liberar_proceso (t_pcb * pcb);
t_tcb* THREAD_CREATE (char* pseudocodigo,int prioridad,int socket_memoria, int pid);
t_cola_prioridad* cola_prioridad(t_list* lista_colas_prioridad, int prioridad);

