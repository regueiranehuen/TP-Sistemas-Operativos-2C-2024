#include "includes/server.h"

t_socket_cpu servidor_CPU_Kernel(t_log* log, t_config* config){

//declaración de variables

char *puerto_dispatch,*puerto_interrupt;
int socket_servidor_Dispatch, socket_servidor_Interrupt;
int socket_cliente_Dispatch =-1, socket_cliente_Interrupt =-1;
int respuesta_Dispatch, respuesta_Interrupt;
t_socket_cpu sockets;
sockets.socket_Dispatch=-1;
sockets.socket_Interrupt=-1;

//Extraccion de datos del config

puerto_dispatch = config_get_string_value(config,"PUERTO_ESCUCHA_DISPATCH");
puerto_interrupt = config_get_string_value(config,"PUERTO_ESCUCHA_INTERRUPT");

// Iniciar servidor

socket_servidor_Dispatch = iniciar_servidor(log,puerto_dispatch);
socket_servidor_Interrupt = iniciar_servidor(log,puerto_interrupt);

sockets.socket_Dispatch=socket_servidor_Dispatch;
sockets.socket_Interrupt=socket_servidor_Interrupt;
   

if (socket_servidor_Dispatch == -1 || socket_servidor_Interrupt == -1) {
    log_error(log, "Error al iniciar el servidor de CPU");
    return sockets;
    }

    log_info(log,"Servidor abierto correctamente");
socket_cliente_Dispatch = esperar_cliente(log,socket_servidor_Dispatch);
socket_cliente_Interrupt = esperar_cliente(log,socket_servidor_Interrupt);

if (socket_cliente_Dispatch == -1 || socket_cliente_Interrupt == -1) {
    log_error(log, "Error al esperar cliente");
    if (socket_cliente_Dispatch != -1) close(socket_cliente_Dispatch);
        if (socket_cliente_Interrupt != -1) close(socket_cliente_Interrupt);
        if (socket_servidor_Dispatch != -1) close(socket_servidor_Dispatch);
        if (socket_servidor_Interrupt != -1) close(socket_servidor_Interrupt);
        return sockets;
    }
respuesta_Dispatch = servidor_handshake(socket_cliente_Dispatch,log);
respuesta_Interrupt = servidor_handshake(socket_cliente_Interrupt,log);

 if (respuesta_Interrupt == 0){
    log_info(log,"Handshake de Kernel --> CPU_Interrupt realizado correctamente");
   }
   else {
    log_error(log, "Handshake de Kernel --> CPU_Interrupt tuvo un error");
   }
   if (respuesta_Dispatch == 0){
    log_info(log,"Handshake de Kernel --> CPU_Dispatch realizado correctamente");
   }
   else {
    log_error(log, "Handshake de Kernel --> CPU_Dispatch tuvo un error");
   }

    close(socket_cliente_Dispatch);
    close(socket_cliente_Interrupt);
	return sockets;
}

t_socket_cpu cliente_cpu_memoria (t_log* log, t_config * config){


char * ip, * puerto_dispatch, * puerto_interrupt;
int socket_Dispatch, socket_Interrupt, respuesta_Dispatch,respuesta_Interrupt;
t_socket_cpu sockets;
sockets.socket_Dispatch=-1;
sockets.socket_Interrupt=-1;

ip = config_get_string_value(config, "IP_MEMORIA");
puerto_dispatch = config_get_string_value(config, "PUERTO_ESCUCHA_DISPATCH");
puerto_interrupt = config_get_string_value(config,"PUERTO_ESCUCHA_INTERRUPT");

 // Verificar que ip y puerto no sean NULL
    if (ip == NULL || puerto_dispatch == NULL || puerto_interrupt ==NULL ) {
        log_info(log, "No se pudo obtener IP o PUERTO de la configuración");
        return sockets;
    }

    // Crear conexión
    socket_Dispatch = crear_conexion(log, ip, puerto_dispatch);
    socket_Interrupt = crear_conexion(log, ip, puerto_interrupt);

    if (socket_Dispatch == -1 || socket_Interrupt == -1) {
        log_info(log, "No se pudo crear la conexión");
        return sockets;
    }

   respuesta_Dispatch = cliente_handshake(socket_Dispatch,log);
   respuesta_Interrupt = cliente_handshake (socket_Interrupt,log);

   if (respuesta_Dispatch == 0){
    log_info(log,"Handshake de CPU_Dispatch --> Memoria realizado correctamente");
   }
   else {
    log_error(log, "Handshake de CPU_Dispatch --> Memoria tuvo un error");
   }
   if (respuesta_Interrupt == 0){
    log_info(log,"Handshake de CPU_Interrupt --> Memoria realizado correctamente");
   }
   else {
    log_error(log, "Handshake de CPU_Interrupt --> Memoria tuvo un error");
   }

sockets.socket_Dispatch=socket_Dispatch;
sockets.socket_Interrupt=socket_Interrupt;

    return sockets;
}

/*
IP_MEMORIA=127.0.0.1
PUERTO_MEMORIA=8002
PUERTO_ESCUCHA_DISPATCH=8006
PUERTO_ESCUCHA_INTERRUPT=8007
LOG_LEVEL=TRACE
*/