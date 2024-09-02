#include "commons/collections/list.h"

typedef struct{
int tid;
int prioridad;
}t_tcb;

typedef struct{
int pid;
t_list* tids;
char* mutex;
}t_pcb;

typedef struct{
t_pcb* pcb;
t_tcb* tcb;
}t_proceso;

t_pcb* crear_pcb();
t_tcb* crear_tcb();
t_proceso iniciar_kernel (char* archivo_pseudocodigo, int tamanio_proceso);