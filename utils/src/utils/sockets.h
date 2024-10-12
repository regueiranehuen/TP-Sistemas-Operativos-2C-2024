#ifndef SOCKETS_H_
#define SOCKETS_H_

#include "utils.h"

// Estructura para pasar argumentos a los hilos
typedef struct {
    t_log* log;
    t_config* config;
} args_hilo;

// Funciones principales de servidor/cliente
int iniciar_servidor(t_log* log, char* puerto);
int esperar_cliente(t_log* log, int socket_servidor);
int crear_conexion(t_log* log, char* ip, char* puerto);
int servidor_handshake(int socket_cliente, t_log* log);
int cliente_handshake(int socket_cliente, t_log* log);
void liberar_conexion(int socket_cliente);
t_config* iniciar_config(char* ruta);

/* Declaraciones comentadas que puedes habilitar si las necesitas más adelante:
void* recibir_buffer(int* size, int socket_cliente);
void recibir_mensaje(t_log* log, int socket_cliente);
int recibir_operacion(int socket_cliente);
t_list* recibir_paquete(int socket_cliente);
void enviar_mensaje(char* mensaje, int socket_cliente);
void eliminar_paquete(t_paquete* paquete);
void crear_buffer(t_paquete* paquete);
void* serializar_paquete(t_paquete* paquete, int bytes);
*/

#endif // SOCKETS_H_