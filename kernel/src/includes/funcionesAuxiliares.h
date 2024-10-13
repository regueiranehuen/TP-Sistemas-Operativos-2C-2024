#ifndef FUNCIONESAUXILIARES_H
#define FUNCIONESAUXILIARES_H

#include "../includes/procesos.h"
#include "../../utils/src/utils/includes/serializacion.h"
#include "commons/string.h"

extern pthread_mutex_t mutex_lista_blocked;

void inicializar_estados();
void destruir_estados();
void inicializar_semaforos();
void destruir_semaforos();
void inicializar_mutex();
void destruir_mutex();
void liberar_proceso (t_pcb * pcb);//libera un proceso
void enviar_tcbs_a_cola_exit_por_pid(t_list* lista_tcbs, t_queue* cola_exit, int pid_buscado);//envia un tcb a la cola exit
t_cola_prioridad* cola_prioridad(t_list* lista_colas_prioridad, int prioridad);//devuelve la cola segun la prioridad
t_mutex* busqueda_mutex(t_list* lista_mutex, char* recurso);//busca un mutex en una lista
void liberar_tcb(t_tcb* tcb);//libera un tcb
bool strings_iguales(char*c1,char*c2);//verifica si dos char son iguales
void ordenar_por_prioridad(t_list* lista);//ordena una lista
void buscar_y_eliminar_tcb(t_list* lista_tcbs, t_tcb* tcb);//elimina un tcb de una lista
t_tcb* buscar_tcb_por_tid(t_list* lista_tcbs, int tid_buscado);//busca un tcb en una lista
t_pcb* buscar_pcb_por_pid(t_list* lista_pcbs, int pid_buscado);//saca un pcb de una lista
t_cola_prioridad* obtener_cola_con_mayor_prioridad(t_list* colas_hilos_prioridad_ready);//devuelve la cola con mayor prioridad con al menos un elemento
t_tcb* sacar_tcb_de_cola(t_queue* cola, t_tcb* tcb_a_sacar);//saca un tcb de una cola
t_tcb* sacar_tcb_de_lista(t_list* lista, t_tcb* tcb_a_sacar);//saca un tcb de una lista

#endif