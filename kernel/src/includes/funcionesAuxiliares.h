#ifndef FUNCIONESAUXILIARES_H
#define FUNCIONESAUXILIARES_H

#include "includes/procesos.h"
#include "includes/serializacion.h"
#include "commons/string.h"



void inicializar_estados_hilos (t_pcb* pcb);
t_pcb* lista_pcb(t_list* lista_pcbs, int pid);
void liberar_proceso (t_pcb * pcb);
t_cola_prioridad* cola_prioridad(t_list* lista_colas_prioridad, int prioridad);
int lista_tcb(t_pcb* pcb, int tid);
int tid_finalizado(t_pcb* pcb, int tid);
t_tcb* find_and_remove_tcb_in_queue(t_queue* queue, int tid);
t_tcb* find_and_remove_tcb_in_list(t_list* list, int tid);
void move_tcb_to_exit(t_queue* queue_new, t_queue* queue_ready, t_list* list_blocked, t_queue* queue_exit, int tid);
t_tcb* find_tcb_in_queue(t_queue* queue, int tid);
t_tcb* find_tcb_in_list(t_list* list, int tid);
t_tcb* buscar_tcb(int tid, t_queue* queue_new, t_queue* queue_ready, t_list* list_blocked);
int suma_tam_hilos_colas_en_lista(t_list*list);
int size_tcbs_queue(t_queue*queue);




#endif