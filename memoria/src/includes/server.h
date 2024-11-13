#ifndef SERVER_H
#define SERVER_H

#include "memoria.h"
#include "memSist.h"
#include "memUsuario.h"
#include "utils/includes/sockets.h"
#include <commons/config.h>
#include <semaphore.h>
#include "utils/includes/serializacion.h"

extern sem_t sem_conexion_iniciales;
extern sem_t sem_conexion_hecha;
extern sem_t sem_fin_memoria;
extern t_list*lista_contextos_pids;

extern pthread_mutex_t mutex_lista_contextos_pids;

extern t_list*lista_instrucciones_tid_pid;


typedef struct{
    int socket_servidor;
    t_log* log;
}hilo_clientes;

typedef struct{//sockets iniciales de kernel y cpu
    int socket_cpu;
    int socket_kernel;
}t_sockets;

typedef struct{

int socket_servidor;
int socket_cliente;

} sockets_memoria;

extern t_sockets* sockets_iniciales;

void* hilo_gestor_clientes (void* void_args);
int servidor_memoria (t_log* log, t_config* config);
int cliente_memoria_filesystem (t_log* log, t_config* config);
void* funcion_hilo_servidor(void* args);
void* funcion_hilo_cliente(void* args);
sockets_memoria* hilos_memoria(t_log* log, t_config* config);







#endif