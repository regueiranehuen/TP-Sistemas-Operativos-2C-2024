#ifndef FUNCIONESAUXILIARES_H
#define FUNCIONESAUXILIARES_H

#include "../includes/procesos.h"
#include "../../utils/src/utils/includes/serializacion.h"
#include "commons/string.h"


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
t_tcb* buscar_tcb_por_tid(t_list* lista_tcbs, int tid_buscado, t_tcb* hilo_exec); //busca un tcb en una lista
t_pcb* buscar_pcb_por_pid(t_list* lista_pcbs, int pid_buscado);//saca un pcb de una lista
t_cola_prioridad* obtener_cola_con_mayor_prioridad(t_list* colas_hilos_prioridad_ready);//devuelve la cola con mayor prioridad con al menos un elemento
t_tcb* sacar_tcb_de_cola(t_queue* cola, t_tcb* tcb_a_sacar);//saca un tcb de una cola
t_tcb* sacar_tcb_de_lista(t_list* lista, t_tcb* tcb_a_sacar);//saca un tcb de una lista
t_tcb* buscar_tcb(int tid_buscado, t_tcb* hilo_exec);
void inicializar_mutex_compartido_entre_procesos(pthread_mutex_t* mutex);
bool esta_en_lista_blocked(t_tcb*tcb_actual);
void sacar_tcbs_de_cola_ready_fifo(t_list* lista_tcbs,t_queue* cola_ready_fifo,int pid_buscado);
void sacar_tcbs_lista_blocked(t_list*lista_bloqueados,int pid_buscado);
t_tcb* buscar_tcb_por_tid_pid(int tid, int pid,t_list* lista_tcbs);
void sacar_tcbs_de_lista_ready_prioridades(t_list* lista_tcbs,t_list* lista_prioridades,int pid_buscado);
t_tcb* sacar_tcb_ready(t_tcb* tcb);
bool tcb_metido_en_estructura(t_tcb*tcb);
void print_queue(t_queue* queue);
void print_lista_prioridades(t_list* lista_prioridades);
void print_lista(t_list* lista);
bool hilo_esta_en_lista(t_list* lista, int tid, int pid);
bool hilo_esta_en_cola(t_queue* cola, int tid, int pid);
bool hilo_esta_en_ready(t_tcb* hilo);
bool hilo_esta_en_colas_multinivel(t_list*colas_ready_prioridad,int tid, int pid, int prioridad);
void eliminar_pcb_lista(t_pcb*pcb,t_list*lista);
void sacar_tcbs_de_colas_ready_multinivel(t_list *lista_prioridades, int pid_buscado);

#endif