#include "includes/server.h"

t_socket_cpu servidor_CPU_Kernel(t_log* log, t_config* config){

char *puerto_dispatch,*puerto_interrupt;
int socket_servidor_Dispatch, socket_servidor_Interrupt, socket_cliente_Dispatch, socket_cliente_Interrupt, respuesta_Dispatch, respuesta_Interrupt;
t_socket_cpu sockets;
puerto_dispatch = config_get_string_value(config,"PUERTO_ESCUCHA_DISPATCH");
puerto_interrupt = config_get_string_value(config,"PUERTO_ESCUCHA_INTERRUPT");

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
socket_cliente_Interrupt = esperar_cliente(log,socket_cliente_Interrupt);

if (socket_cliente_Dispatch == -1 || socket_cliente_Interrupt == -1) {
    log_error(log, "Error al esperar cliente");
    close(socket_servidor_Dispatch);
    close(socket_cliente_Interrupt);
    return sockets;
}    
respuesta_Dispatch = servidor_handshake(socket_cliente_Dispatch,log);
respuesta_Interrupt = servidor_handshake(socket_cliente_Interrupt,log);

 if (respuesta_Interrupt == 0){
    log_info(log,"Handshake de CPU_Interrupt --> Kernel realizado correctamente");
   }
   else {
    log_error(log, "Handshake de CPU_Interrupt --> Kernel tuvo un error");
   }
   if (respuesta_Dispatch == 0){
    log_info(log,"Handshake de CPU_Dispatch --> Kernel realizado correctamente");
   }
   else {
    log_error(log, "Handshake de CPU_Dispatch --> Kernel tuvo un error");
   }

    close(socket_cliente_Dispatch);
    close(socket_cliente_Interrupt);
	return sockets;
}


