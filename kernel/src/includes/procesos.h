#ifndef PROCESOS_H
#define PROCESOS_H


#include "commons/collections/list.h"
#include "commons/collections/queue.h"
#include "utils/includes/sockets.h"
#include "../includes/cliente.h"
#include <semaphore.h>

extern t_queue* cola_new_procesos;
extern t_queue* cola_exit_procesos;

extern t_queue* cola_ready_fifo;
extern t_list* lista_ready_prioridad;
extern t_list* colas_ready_prioridad;
extern t_list* lista_bloqueados;
extern t_queue* cola_exit;
extern t_queue* cola_IO;

extern t_list* lista_tcbs;
extern t_list* lista_pcbs;
extern t_list* lista_mutex;

extern t_config* config;
extern t_log* logger;
extern sockets_kernel *sockets;

extern pthread_mutex_t mutex_lista_pcbs;
extern pthread_mutex_t mutex_lista_tcbs;
extern pthread_mutex_t mutex_cola_new_procesos;
extern pthread_mutex_t mutex_cola_exit_procesos;
extern pthread_mutex_t mutex_cola_exit_hilos;
extern pthread_mutex_t mutex_cola_ready;
extern pthread_mutex_t mutex_conexion_kernel_a_dispatch;
extern pthread_mutex_t mutex_conexion_kernel_a_interrupt;
extern pthread_mutex_t mutex_log;
extern pthread_mutex_t mutex_lista_blocked;
extern pthread_mutex_t mutex_syscall_ejecutando;


extern sem_t semaforo_new_ready_procesos;
extern sem_t semaforo_cola_new_procesos;
extern sem_t semaforo_cola_exit_procesos;
extern sem_t sem_desalojado;

extern sem_t semaforo_cola_ready;

extern sem_t sem_cola_IO;

extern sem_t semaforo_cola_exit_hilo_exec_process_exit;
extern sem_t semaforo_cola_exit_hilos_process_exit;
extern sem_t semaforo_cola_exit_hilos;

extern sem_t sem_lista_prioridades;

extern sem_t sem_fin_kernel;

extern sem_t sem_fin_syscall;

extern bool desalojado;
extern bool syscallEjecutando;

extern int pipe_fds[2]; // pipe_fds[0] para leer, pipe_fds[1] para escribir



typedef enum {
    TCB_NEW,
    TCB_EXECUTE,
    TCB_READY,
    TCB_BLOCKED,
    TCB_BLOCKED_MUTEX,
    TCB_EXIT
} estado_hilo;


typedef struct{
int tid;
int prioridad;
int pid; // proceso asociado al hilo
estado_hilo estado;
char* pseudocodigo;
int pseudocodigo_length;
t_queue* cola_hilos_bloqueados;
pthread_mutex_t mutex_cola_hilos_bloqueados;
bool desalojo;
}t_tcb;

typedef struct{
    t_tcb* hilo;
    int milisegundos;
} t_nodo_cola_IO;

extern t_tcb* hilo_exec;

typedef enum {
    UNLOCKED,
    LOCKED
} estado_mutex;

typedef enum{
PCB_NEW,
PCB_READY,
PCB_BLOCKED,
PCB_EXECUTE,
PCB_EXIT
}estado_pcb;

typedef struct{

int pid;
t_list* tids;
t_list* lista_mutex;
estado_pcb estado;
int tamanio_proceso;

t_tcb* tcb_main;

pthread_mutex_t mutex_lista_mutex;
pthread_mutex_t mutex_tids;

int contador_tid; 
int contador_mutex;

}t_pcb;

typedef struct {
    int prioridad;
    t_queue* cola;
} t_cola_prioridad;

typedef struct{
int mutex_id;
t_queue* cola_tcbs;
estado_mutex estado;
t_tcb* hilo; // hilo que esta en la región crítica
char* nombre;
}t_mutex;

t_pcb* crear_pcb();
t_tcb* crear_tcb(t_pcb *pcb);

void PROCESS_CREATE (char* pseudocodigo,int tamanio_proceso,int prioridad);
void PROCESS_EXIT();

void THREAD_CREATE (char* pseudocodigo,int prioridad);
void THREAD_JOIN (int tid);
void THREAD_CANCEL(int tid);
void THREAD_EXIT();

void new_a_ready_procesos();
void proceso_exit();
void hilo_exit();
void hilo_exec_exit_tras_process_exit();

void iniciar_kernel (char* archivo_pseudocodigo, int tamanio_proceso);

void MUTEX_CREATE(char* recurso);
void MUTEX_LOCK(char* recurso);
void MUTEX_UNLOCK(char* recurso);

void IO(int milisegundos);

void DUMP_MEMORY();

void dispositivo_IO();
void* hilo_dispositivo_IO();





#endif