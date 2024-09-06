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

void* funcion_hilo_servidor(void* void_args){
    
    args_hilo* args = ((args_hilo*)void_args);


    int socket_servidor = servidor_FileSystem_Memoria(args->log, args->config);
    if (socket_servidor == -1) {
        log_error(args->log, "No se pudo establecer la conexion con Memoria");
        pthread_exit(NULL);
    }

   return (void*)(intptr_t)socket_servidor;
}

int hilo_filesystem(t_log* log, t_config* config){

pthread_t hilo_servidor;

args_hilo* args = malloc(sizeof(args_hilo)); 

args->config=config;
args->log=log;

void* socket_servidor;

int resultado;

resultado = pthread_create (&hilo_servidor,NULL,funcion_hilo_servidor,(void*)args);

if(resultado != 0){
    log_error(log,"Error al crear el hilo");
    free (args);
    return -1;
}

log_info(log,"El hilo servidor se creo correctamente");

pthread_join(hilo_servidor,&socket_servidor);

resultado = (intptr_t)socket_servidor;

free(args);
return resultado;
}
