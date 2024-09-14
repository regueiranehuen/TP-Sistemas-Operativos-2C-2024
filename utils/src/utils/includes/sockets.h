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

typedef enum
{
	MENSAJE,
	PAQUETE,
    SET,
    READ_MEM,
    WRITE_MEM,
    SUM,
    SUB,
    JNZ,
    LOG,
    DUMP_MEMORY,
    IO,
    PROCESS_CREATE,
    THREAD_CREATE,
    THREAD_JOIN,
    THREAD_CANCEL,
    MUTEX_CREATE,
    MUTEX_LOCK,
    MUTEX_UNLOCK,
    THREAD_EXIT,
    PROCESS_EXIT
}op_code;

typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

typedef struct{
t_log* log;
t_config* config;
} args_hilo;

//Registros de CPU
typedef struct{
    uint32_t AX;
    uint32_t BX;
    uint32_t CX;
    uint32_t DX;
	uint32_t EX;
    uint32_t FX;
    uint32_t GX;
    uint32_t HX;
}t_registros_cpu;

typedef struct{
    char* parametros1;
    char* parametros2;
    char* parametros3;
    char* parametros4;
    char* parametros5;
    char* parametros6;
}t_instruccion;

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