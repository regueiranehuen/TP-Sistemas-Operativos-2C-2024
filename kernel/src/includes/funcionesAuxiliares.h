#ifndef FUNCIONESAUXILIARES_H
#define FUNCIONESAUXILIARES_H

#include "../includes/procesos.h"
#include "../../utils/src/utils/includes/serializacion.h"
#include "commons/string.h"


t_pcb* lista_pcb(t_list* lista_pcbs, int pid);
void liberar_proceso (t_pcb * pcb);
t_cola_prioridad* cola_prioridad(t_list* lista_colas_prioridad, int prioridad);
int lista_tcb(t_pcb* pcb, int tid);
int tid_finalizado(t_pcb* pcb, int tid);
t_tcb* find_and_remove_tcb_in_queue(t_queue* queue, int tid);
t_tcb* find_and_remove_tcb_in_list(t_list* list, int tid);
void move_tcb_to_exit(t_pcb* pcb, t_tcb* tcb);
t_tcb* find_tcb_in_queue(t_queue* queue, int tid);
t_tcb* find_tcb_in_list(t_list* list, int tid);
t_tcb* buscar_tcb(int tid, t_queue* queue_new, t_list* queue_ready, t_list* list_blocked);
t_mutex* busqueda_mutex(t_list* lista_mutex, int mutex_id);
int suma_tam_hilos_colas_en_lista(t_list*list);
int size_tcbs_queue(t_queue* queue);
int tam_tcb(t_tcb * tcb);
int tam_pcb(t_tcb* pcb);
void liberar_tcb(t_tcb* tcb);
t_tcb* buscar_tcb_por_tid(t_pcb* pcb, int tid);
void insertar_ordenado(t_queue*cola, t_tcb* nuevo_hilo);
bool strings_iguales(char*c1,char*c2);
bool es_motivo_devolucion(code_operacion motivo_devolucion);
void* ordenar_cola(void*arg);
void ordenar_por_prioridad(t_list* lista);
int obtener_menor_prioridad(t_list* lista_cola_prioridad);
t_cola_prioridad* obtener_cola_por_prioridad(t_list *colas_hilos_prioridad_ready, int prioridad_buscada);
void ordenar_por_prioridad(t_list* lista);
void inicializar_mutex_procesos(t_pcb* pcb);
void destruir_mutex_procesos(t_pcb* pcb);
void inicializar_mutex_hilo(t_tcb* tcb);
void destruir_mutex_hilo(t_tcb* tcb);

#endif