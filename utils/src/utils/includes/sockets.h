#ifndef SOCKETS_H_
#define SOCKETS_H_

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/collections/list.h>
#include<string.h>
#include<assert.h>
#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<commons/log.h>
#include<commons/config.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>

typedef struct{
t_log* log;
t_config* config;
} args_hilo;

int iniciar_servidor(t_log* log,char* puerto);
int esperar_cliente(t_log* log,int socket_servidor);
int crear_conexion(t_log* log,char *ip, char* puerto);
/*void* recibir_buffer(int* size, int socket_cliente);
void recibir_mensaje(t_log* log,int socket_cliente);
int recibir_operacion(int socket_cliente);
t_list* recibir_paquete(int socket_cliente);
void enviar_mensaje(char* mensaje, int socket_cliente);
void eliminar_paquete(t_paquete* paquete);
void crear_buffer(t_paquete* paquete);
void* serializar_paquete(t_paquete* paquete, int bytes);
*/
int servidor_handshake(int socket_cliente, t_log* log);
int cliente_handshake(int socket_cliente, t_log* log);
#endif 