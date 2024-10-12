#ifndef SERVER_H
#define SERVER_H

#include "utils/sockets.h"

//Variables y structs
typedef struct{
    int socket_servidor;
    t_log* log;
}hilo_clientes;
typedef struct{

int socket_servidor;
int socket_cliente;

} sockets_memoria;

t_log* log_memoria;
t_config* config;
int puerto_escucha;
char* ip_file;
int puerto_file;
int tam_memoria;
char* path_instrucciones;
int retardo_respuesta;
char* esquema;
char* algoritmo;
char** particiones;
char* log_level;
void* socket_cliente;
void* socket_servidor;

pthread_t hilo_servidor;
pthread_t hilo_cliente;

int socket_servidor;
int socket_cliente;

pthread_t hilo_socket_1;
pthread_t hilo_socket_2;

//funciones
void leer_config(char* path_config);
void* hilo_gestor_clientes (void* void_args);
int servidor_memoria_kernel (t_log* log, t_config* config);
int cliente_memoria_filesystem (t_log* log, t_config* config);
void* funcion_hilo_servidor(void* args);
void* funcion_hilo_cliente(void* args);
sockets_memoria* hilos_memoria(t_log* log, t_config* config);





#endif