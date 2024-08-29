#include "includes/server.h"

int servidor_FileSystem_Memoria(t_log* log, t_config* config){

char * puerto;
int socket_servidor, socket_cliente, respuesta_Handshake;

puerto = config_get_string_value(config,"PUERTO_ESCUCHA");

socket_servidor = iniciar_servidor (log,puerto);

if(socket_servidor ==-1){
    log_error(log, "Error al iniciar el servidor de Filesystem");
    return socket_servidor;
}

log_info(log,"Servidor abierto correctamente");

socket_cliente = esperar_cliente(log,socket_servidor);

if (socket_cliente ==-1){
 log_error(log, "Error al esperar al cliente");
 close (socket_cliente);
    return socket_servidor;
}

respuesta_Handshake = servidor_handshake(socket_cliente,log);

if (respuesta_Handshake == 0){
    log_info(log,"Handshake de Memoria --> Filesystem realizado correctamente");
   }
   else {
    log_error(log, "Handshake de Memoria --> Filesystem tuvo un error");
   }

close(socket_cliente);
return socket_servidor;
}