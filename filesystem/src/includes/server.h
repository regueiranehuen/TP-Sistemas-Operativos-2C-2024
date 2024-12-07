#ifndef SERVER_H
#define SERVER_H

#include "utils/includes/sockets.h"
#include "utils/includes/serializacion.h"
#include "utils/includes/estructuras.h"
#include <semaphore.h>
#include "main.h"
#include "auxiliares.h"

typedef struct{
    int socket_servidor;
    t_log* log;
}hilo_clientes;

extern int estado_filesystem;

extern sem_t sem_conexion_hecha;

void* hilo_por_cliente(void* void_args);
void* gestor_clientes(void* void_args);
int servidor_FileSystem_Memoria(t_log* log, t_config* config);
void* funcion_hilo_servidor(void* void_args);
int hilo_filesystem(t_log* log, t_config* config);

#endif