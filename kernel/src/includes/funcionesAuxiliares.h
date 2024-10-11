#ifndef FUNCIONESAUXILIARES_H
#define FUNCIONESAUXILIARES_H

#include "../includes/procesos.h"
#include "../../utils/src/utils/includes/serializacion.h"
#include "commons/string.h"


t_pcb* lista_pcb(t_list* lista_pcbs, int pid);
void liberar_proceso (t_pcb * pcb);
t_cola_prioridad* cola_prioridad(t_list* lista_colas_prioridad, int prioridad);
int lista_tcb(t_pcb* pcb, int tid);
//int tid_finalizado(t_pcb* pcb, int tid);
t_tcb* find_and_remove_tcb_in_queue(t_queue* queue, int tid);
t_tcb* find_and_remove_tcb_in_list(t_list* list, int tid);
void move_tcb_to_exit(t_tcb* tcb, t_queue* cola_new, t_queue* cola_ready_fifo, t_list* lista_ready_prioridades, t_list* colas_ready_prioridades, t_list* lista_blocked);
t_tcb* find_tcb_in_queue(t_queue* queue, int tid);
t_tcb* find_tcb_in_list(t_list* list, int tid);
t_tcb* buscar_tcb(int tid, t_queue* queue_new, t_list* queue_ready, t_list* list_blocked);
t_mutex* busqueda_mutex(t_list* lista_mutex, char* recurso);
int suma_tam_hilos_colas_en_lista(t_list*list);
int size_tcbs_queue(t_queue* queue);
int tam_tcb(t_tcb * tcb);
int tam_pcb(t_tcb* pcb);
void liberar_tcb(t_tcb* tcb);
void insertar_ordenado(t_queue*cola, t_tcb* nuevo_hilo);
bool strings_iguales(char*c1,char*c2);
bool es_motivo_devolucion(code_operacion motivo_devolucion);
void* ordenar_cola(void*arg);
void ordenar_por_prioridad(t_list* lista);
void inicializar_mutex_procesos(t_pcb* pcb);
void destruir_mutex_procesos(t_pcb* pcb);
void inicializar_mutex_hilo(t_tcb* tcb);
void destruir_mutex_hilo(t_tcb* tcb);
void buscar_y_eliminar_tcb(t_list* lista_tcbs, t_tcb* tcb);
t_tcb* buscar_tcb_por_tid(t_list* lista_tcbs, int tid_buscado);
t_tcb* sacar_tcb_por_tid(t_list* lista_tcbs, int tid_buscado);
t_pcb* buscar_pcb_por_pid(t_list* lista_pcbs, int pid_buscado);
void enviar_tcbs_a_cola_exit_por_pid(t_list* lista_tcbs, t_queue* cola_exit, int pid_buscado);
t_cola_prioridad* obtener_cola_con_mayor_prioridad(t_list* colas_hilos_prioridad_ready);
t_tcb* sacar_tcb_de_cola(t_queue* cola, t_tcb* tcb_a_sacar);
t_tcb* sacar_tcb_de_lista(t_list* lista, t_tcb* tcb_a_sacar);
void pushear_cola_ready(t_tcb* hilo);

#endif