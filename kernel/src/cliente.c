#include "includes/cliente.h"

int cliente_Memoria_Kernel(t_log* log, t_config* config) {
    char* ip;
    char* puerto;
    int socket_cliente, respuesta;

    // Asignar valores a las variables ip y puerto usando config_get_string_value
    ip = config_get_string_value(config, "IP_MEMORIA");
    puerto = config_get_string_value(config, "PUERTO_MEMORIA");

    // Verificar que ip y puerto no sean NULL
    if (ip == NULL || puerto == NULL) {
        log_info(log, "No se pudo obtener IP o PUERTO de la configuración");
        return -1;
    }

    // Crear conexión
    socket_cliente = crear_conexion(log, ip, puerto);
    if (socket_cliente == -1) {
        log_info(log, "No se pudo crear la conexión");
        return -1;
    }

   respuesta = cliente_handshake(socket_cliente,log);
   if (respuesta == 0){
    log_info(log,"Handshake de Kernel --> Memoria realizado correctamente");
   }
   else {
    log_error(log, "Handshake de Kernel --> Memoria tuvo un error");
   }

    return socket_cliente;
}

t_socket_cpu cliente_CPU_Kernel(t_log* log, t_config* config){

    char* ip;
    char* puerto_Dispatch,puerto_Interrupt;
    int socket_cliente_Interrupt,socket_cliente_Dispatch, respuesta_Dispatch, respuesta_Interrupt;
    t_socket_cpu resultado;

    // Asignar valores a las variables ip y puerto usando config_get_string_value
    ip = config_get_string_value(config, "IP_CPU");
    puerto_Dispatch = config_get_string_value(config, "PUERTO_CPU_DISPATCH");
    puerto_Interrupt= config_get_string_value(config, "PUERTO_CPU_INTERRUPT"); 

    // Verificar que ip y puerto no sean NULL
    if (ip == NULL || puerto_Dispatch == NULL || puerto_Interrupt == NULL) {
        log_info(log, "No se pudo obtener IP o PUERTO de la configuración");
        resultado.socket_Dispatch = -1;
        resultado.socket_Interrupt = -1;
        return resultado;
    }

    // Crear conexión
    socket_cliente_Interrupt = crear_conexion(log, ip, puerto_Interrupt);
    socket_cliente_Dispatch = crear_conexion(log,ip,puerto_Dispatch);
    if (socket_cliente_Interrupt == -1 || socket_cliente_Dispatch == -1) {
        log_info(log, "No se pudo crear la conexión");
        resultado.socket_Dispatch = -1;
        resultado.socket_Interrupt = -1;
        return resultado;
    }

   respuesta_Interrupt = cliente_handshake(socket_cliente_Interrupt,log);
   respuesta_Dispatch = cliente_handshake(socket_cliente_Dispatch,log);
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

   resultado.socket_Dispatch=socket_cliente_Dispatch;
   resultado.socket_Interrupt=socket_cliente_Interrupt;

    return resultado;
}

