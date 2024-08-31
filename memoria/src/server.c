#include "includes/server.h"

int servidor_memoria_kernel (t_log* log, t_config* config){

char * puerto;
int socket_servidor,socket_cliente, respuesta_Handshake;

puerto = config_get_string_value(config,"PUERTO_ESCUCHA");

socket_servidor = iniciar_servidor(log,puerto);

if (socket_servidor == -1) {
    log_error(log, "Error al iniciar el servidor");
    return -1;
    }

    log_info(log,"Servidor abierto correctamente");

socket_cliente = esperar_cliente(log,socket_servidor);

if (socket_cliente == -1) {
    log_error(log, "Error al esperar cliente");
    close(socket_servidor);
    return -1;
}

respuesta_Handshake = servidor_handshake(socket_cliente,log);

if (respuesta_Handshake == 0){
    log_info(log,"Handshake de Kernel --> Memoria realizado correctamente");
   }
   else {
    log_error(log, "Handshake de Kernel --> Memoria tuvo un error");
   }

    close(socket_cliente);
	return socket_servidor;
}

void* función_hilo_servidor(void *void_args){
    
    args_hilo *args = (args_hilo*)void_args;

    int socket_servidor = servidor_memoria_kernel(args->log,args->config);
    if (socket_servidor == -1) {
        log_error(args->log, "No se pudo establecer la conexión con la Memoria");
        pthread_exit(NULL);
    }

   return (void*)(intptr_t)socket_servidor;
}

int cliente_memoria_filesystem (t_log* log, t_config* config){

char * ip, * puerto;
int socket_cliente, respuesta;

ip = config_get_string_value(config, "IP_FILESYSTEM");
puerto = config_get_string_value(config, "PUERTO_FILESYSTEM");

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
    log_info(log,"Handshake de Memoria --> Filesystem realizado correctamente");
   }
   else {
    log_error(log, "Handshake de Memoria --> Filesystem tuvo un error");
   }

    return socket_cliente;

}

void* función_hilo_cliente(void* void_args){
    
    args_hilo* args = ((args_hilo*)void_args);


    int socket_cliente = cliente_memoria_filesystem(args->log,args->config);
    if (socket_cliente == -1) {
        log_error(args->log, "No se pudo establecer la conexión con filesystem");
        pthread_exit(NULL);
    }

   return (void*)(intptr_t)socket_cliente;
}

sockets_memoria* hilos_memoria(t_log* log, t_config* config){

pthread_t hilo_servidor;
pthread_t hilo_cliente;

args_hilo* args = malloc(sizeof(args_hilo)); 

args->config=config;
args->log=log;

void* socket_cliente;
void* socket_servidor;

int resultado;

sockets_memoria* sockets=malloc(sizeof(sockets_memoria));


resultado = pthread_create (&hilo_servidor,NULL,función_hilo_servidor,(void*)args);

if(resultado != 0){
    log_error(log,"Error al crear el hilo");
    free (args);
    return NULL;
}

log_info(log,"El hilo servidor se creo correctamente");

resultado = pthread_create (&hilo_cliente,NULL,función_hilo_cliente,(void*)args);

if(resultado != 0){
    log_error(log,"Error al crear el hilo");
    free (args);
    return NULL;
}

log_info(log,"El hilo cliente se creo correctamente");

pthread_join(hilo_cliente,&socket_cliente);
pthread_join(hilo_servidor,&socket_servidor);

resultado = (intptr_t)socket_cliente;

sockets->socket_cliente=resultado;

resultado = (intptr_t)socket_servidor;

sockets->socket_servidor = resultado;

free(args);
return sockets;
}
