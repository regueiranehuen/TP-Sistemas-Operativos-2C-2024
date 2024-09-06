#ifndef SERVER_H
#define SERVER_H

#include <utils/includes/sockets.h>
#include <commons/config.h>

typedef struct{
    int socket_servidor;
    t_log* log;
}hilo_clientes;
typedef struct{

int socket_servidor;
int socket_cliente;

} sockets_memoria;

void* hilo_gestor_clientes (void* void_args);
int servidor_memoria_kernel (t_log* log, t_config* config);
int cliente_memoria_filesystem (t_log* log, t_config* config);
void* funcion_hilo_servidor(void* args);
void* funcion_hilo_cliente(void* args);
sockets_memoria* hilos_memoria(t_log* log, t_config* config);





#endif