#include "includes/procesos.h"

sem_t semaforo;

t_pcb *crear_pcb()
{
    static int pid = 0;
    t_pcb *pcb = malloc(sizeof(t_pcb));
    t_list *lista_tids = list_create();
    pcb->pid = pid;
    pcb->tids = lista_tids;
    pid += 1;
    return pcb;
}

t_tcb *crear_tcb(t_pcb *pcb)
{

    t_tcb *tcb = malloc(sizeof(t_tcb));
    int tamanio_lista = list_size(pcb->tids);
    tcb->tid = tamanio_lista;
    list_add(pcb->tids, tcb->tid);
    if (tcb->tid == 0)
    {
        tcb->prioridad = 0;
    }
    return tcb;
}

void crear_proceso(t_queue *cola_new_procesos, int socket_memoria, t_queue *cola_ready_proceso)
{
    t_pcb *pcb = crear_pcb();
    queue_push(cola_new_procesos, pcb);
    new_a_ready(cola_new_procesos, socket_memoria, cola_ready_proceso);
    pcb->estado = "NEW";
}

void new_a_ready(t_queue *cola_new_procesos, int socket_memoria, t_queue *cola_ready_proceso) // Crear pcb, verificar contra la memoria si se puede inicializar, si es asi se inicia el hilo
{
    int pedido = 1;
    t_pcb *pcb = queue_pop(cola_new_procesos);


    // Hacer serializacion del tipo pcb
    send(socket_memoria, pcb, sizeof(t_pcb), 0); // Enviar pcb para que memoria verifique si tiene espacio para inicializar el proximo proceso
    recv(socket_memoria, &pedido, sizeof(int), 0);
    queue_push(pcb, cola_new_procesos);
    if (pedido == -1)
    {
        sem_wait(&semaforo);
    }
    else
    {
        pcb = queue_pop(cola_new_procesos);
        t_tcb *tcb = crear_tcb(pcb);
        pcb->estado = "READY";
        queue_push(cola_ready_proceso, pcb);
    }
}

void finalizar_proceso(t_pcb *pcb, int socket_memoria)
{
    int pedido;
    send(socket_memoria,pcb,sizeof(t_pcb),0);
    recv(socket_memoria,&pedido,sizeof(int),0);
    liberar_pcb(pcb);
    sem_post(&semaforo);
}

t_proceso iniciar_kernel(char *archivo_pseudocodigo, int tamanio_proceso)
{
    t_pcb *pcb = crear_pcb();
    t_tcb *tcb = crear_tcb(pcb);
    t_proceso proceso;
    proceso.pcb = pcb;
    proceso.tcb = tcb;
    return proceso;
}


void liberar_pcb(t_pcb *pcb)
{
    list_destroy(pcb->tids);
    free(pcb);
}

