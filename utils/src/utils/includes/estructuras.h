#ifndef UTILS_H_
#define UTILS_H_

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <commons/log.h> 
#include <commons/config.h>
#include <commons/bitarray.h>
#include <commons/temporal.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <unistd.h>
#include <netdb.h>
#include <assert.h>
#include <math.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <pthread.h>
#include <arpa/inet.h>
#include "serializacion.h"

typedef struct{
    uint32_t AX;
    uint32_t BX;
    uint32_t CX;
    uint32_t DX;
	uint32_t EX;
    uint32_t FX;
    uint32_t GX;
    uint32_t HX;
    uint32_t base;
    uint32_t limite;
}t_registros_cpu;

typedef struct {
	uint32_t tid;
	uint32_t pc;
	t_registros_cpu* registros;
}t_contexto;

typedef struct {
	t_contexto* contexto;
	int quantum_utilizado;
	t_temporal* quantum;
}t_pcb;

typedef enum {
    SUCCESS,
    INVALID_RESOURCE,
    INVALID_INTERFACE,
    OUT_OF_MEMORY,
    INTERRUPTED_BY_USER,
}motivo_exit;

typedef struct {
    t_pcb* pcb;
    motivo_exit motivo;
}t_pcb_exit;

typedef enum
{
    Algo, //hay un case -1 que lo cambie a 1
    //ESTADOS
    NEW,
    READY,
    EXEC,
    BLOCK,
    EXIT_,
    //MENSAJES GENERICOS
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
    PROCESS_EXIT,
    WAIT,
    SIGNAL,
    EXIT,
    SEGMENTATION_FAULT,
    //SOLICITUDES DE CPU A OTROS
    PEDIR_INSTRUCCION_MEMORIA,
    //SOLICITUDES DE KERNEL A OTROS
    INICIO_NUEVO_PROCESO,
    FINALIZO_PROCESO,
    FIN_QUANTUM_RR_CPU,
    //COSAS QUE LE LLEGAN EL KERNEL
    IDENTIFICACION,
    OBTENER_VALIDACION,
    //MOTIVOS DE DESALOJO
    TERMINO_PROCESO,
    INTERRUPCION,
    INTERRUPCION_USUARIO,
    ERROR,
    LLAMADA_POR_INSTRUCCION,
    //COMUNICACION MEMORIA CON MODULOS
    CREAR_PROCESO,
    ACCESO,
    FINALIZAR_PROCESO,
    ACCESO_TABLA_PAGINAS,
    ACCESO_ESPACIO_USUARIO,
    //INSTRUCCION FINALIZADA
    PEDIR_TAM_MEMORIA,
    TAMANIO_RECIBIDO,
    ACTUALIZAR_CONTEXTO_MEMORIA,
    TERMINACION_PROCESO,
    OBTENER_CONTEXTO,
    ACTUALIZAR_CONTEXTO,
    OBTENER_INSTRUCCION,
    ESPACIO_USUARIO,
    WRITE_OK,
    ACTUALIZACION_OK,
}op_code;


typedef struct{
    char* parametros1;
    char* parametros2;
    char* parametros3;
    char* parametros4;
    char* parametros5;
    char* parametros6;
}t_instruccion;


typedef struct {
    uint32_t entero1;
    uint32_t entero2;
}t_2_enteros;

typedef struct {
    uint32_t entero;
    bool operacion;
}t_entero_bool;

typedef struct {
    uint32_t entero1;
    uint32_t entero2;
    uint32_t entero3;
}t_3_enteros;

typedef struct {
    uint32_t entero1;
    uint32_t entero2;
    uint32_t entero3;
    uint32_t entero4;
}t_4_enteros;

typedef struct {
    char *string;
    uint32_t entero1;
    uint32_t entero2;
}t_string_2enteros;

typedef struct {
    char *string;
    uint32_t entero1;
    int entero2;
}t_string_2enteros_dato_movOut;

typedef struct {
    char *string;
    uint32_t entero1;
    uint32_t entero2;
	uint32_t entero3;
}t_string_3enteros;

typedef struct {
    char *string;
    uint32_t entero1;
}t_string_mas_entero;

typedef struct
{
    op_code codigo_operacion;
    t_buffer* buffer;
} t_paquete; // cpu/memoria

int recibir_operacion(int socket_cliente);

void* recibir_buffer(int* size, int socket_cliente);
void recibir_mensaje(int socket_cliente, t_log* loggs);
t_list* recibir_paquete(int socket_cliente);
void* serializar_paquete(t_paquete* paquete, int bytes);
void enviar_mensaje(char* mensaje, int socket_cliente);
void crear_buffer(t_paquete* paquete);
t_paquete* crear_paquete(void);
void agregar_a_paquete(t_paquete* paquete, void* valor, uint32_t tamanio);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
void eliminar_paquete(t_paquete* paquete);
void eliminar_codigo(t_paquete* codop);
void liberar_conexion(int socket_cliente);
t_config* iniciar_config(char *ruta);
char* obtener_instruccion(uint32_t pid, uint32_t pc);



// Serializacion

void agregar_entero_a_paquete(t_paquete *paquete, uint32_t numero);
void agregar_entero_uint8_a_paquete(t_paquete *paquete, uint8_t numero);
void agregar_entero_int_a_paquete(t_paquete *paquete, int numero);
void agregar_string_a_paquete(t_paquete *paquete, char* palabra);
void agregar_contexto_a_paquete(t_paquete *paquete, t_contexto * pcb);
void agregar_registros_a_paquete(t_paquete * paquete, t_registros_cpu * registros);
void agregar_instruccion_a_paquete(t_paquete *paquete, t_instruccion * instruccion_nueva);
void agregar_2_enteros_1_string_a_paquete(t_paquete *paquete, t_string_2enteros * enteros_string);
void agregar_2_enteros_a_paquete(t_paquete *paquete, t_2_enteros * enteros);
void agregar_3_enteros_a_paquete(t_paquete *paquete, t_3_enteros * enteros);
void agregar_3_enteros_1_string_a_paquete(t_paquete *paquete, t_string_3enteros * enteros_string);
void enviar_entero (int conexion, uint32_t numero, int codop);
void enviar_string (int conexion, char* palabra, int codop);
void enviar_contexto (int conexion, t_contexto* pcb, int codop);
void enviar_instruccion (int conexion, t_instruccion* nueva_instruccion, int codop);
void enviar_2_enteros(int conexion, t_2_enteros* enteros, int codop);
void enviar_3_enteros(int conexion, t_3_enteros* enteros, int codop);
void enviar_2_enteros_1_string(int conexion, t_string_2enteros* enteros_string, int codop);
void enviar_3_enteros_1_string(int conexion, t_string_3enteros* enteros_string, int codop);
void enviar_codigo (t_paquete *codop, int socket_cliente);
void enviar_codop(int conexion, op_code cod_op);
void enviar_paquete_string(int conexion, char* string, op_code codOP, int tamanio);

t_paquete* crear_paquete_op(op_code codop);


// Una vez serializado -> recibimos y leemos estas variables

int leer_entero(char *buffer, int * desplazamiento);
uint8_t leer_entero_uint8(char *buffer, int * desplazamiento);
uint32_t leer_entero_uint32(char *buffer, int * desplazamiento);
char* leer_string(char *buffer, int * desplazamiento);
t_registros_cpu * leer_registros(char* buffer, int* desp);

uint32_t recibir_entero_uint32(int socket, t_log* loggs);
char* recibir_string(int socket, t_log* loggs);
t_contexto* recibir_contexto(int socket);
t_instruccion* recibir_instruccion(int socket);
t_list* recibir_doble_entero(int socket);

void recibir_string_mas_contexto(int conexion_kernel_cpu_dispatch,t_contexto** pcb_wait,char** recurso_wait);
void recibir_string_mas_u32_con_contexto(int conexion_kernel_cpu_dispatch,char** palabra,uint32_t* numero,t_contexto** contexto);
void recibir_3_string(int conexion_kernel_cpu_dispatch, char** palabra1,char** palabra2, char** palabra3);
void recibir_2_string_con_contexto(int conexion_kernel_cpu_dispatch, char** palabra1,char** palabra2, t_contexto** contexto);
t_string_2enteros* recibir_string_2enteros_con_contexto(int socket, t_contexto** contexto);
t_2_enteros * recibir_2_enteros(int socket);
t_3_enteros* recibir_3_enteros(int socket);
t_4_enteros* recibir_4_enteros(int socket);
t_string_3enteros* recibir_string_3_enteros(int socket);
t_string_2enteros* recibir_string_2enteros(int socket);

t_string_mas_entero* recibir_string_mas_entero(int socket, t_log *loggs);
void recibir_2_string_mas_u32(int socket, char** palabra1,char** palabra2, uint32_t* valor1);
void recibir_2_string_mas_3_u32(int socket, char** palabra1,char** palabra2, uint32_t* valor1, uint32_t* valor2, uint32_t* valor3);
void recibir_2_string_mas_u32_con_contexto(int socket, char** palabra1,char** palabra2, uint32_t* valor1, t_contexto** contexto);
void recibir_2_string_mas_3_u32_con_contexto(int socket, char** palabra1,char** palabra2, uint32_t* valor1, uint32_t* valor2, uint32_t* valor3, t_contexto** contexto);



#endif