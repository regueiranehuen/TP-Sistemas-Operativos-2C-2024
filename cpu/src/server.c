#include "includes/server.h"

t_socket_cpu* servidor_CPU_Kernel(t_log* log, t_config* config){

//declaración de variables

char *puerto_dispatch,*puerto_interrupt;
int socket_servidor_Dispatch, socket_servidor_Interrupt;
int socket_cliente_Dispatch =-1, socket_cliente_Interrupt =-1;
int respuesta_Dispatch, respuesta_Interrupt;
t_socket_cpu* sockets=malloc(sizeof(t_socket_cpu));
sockets->socket_Dispatch=-1;
sockets->socket_Interrupt=-1;

//Extraccion de datos del config

puerto_dispatch = config_get_string_value(config,"PUERTO_ESCUCHA_DISPATCH");
puerto_interrupt = config_get_string_value(config,"PUERTO_ESCUCHA_INTERRUPT");

// Iniciar servidor

socket_servidor_Dispatch = iniciar_servidor(log,puerto_dispatch);
socket_servidor_Interrupt = iniciar_servidor(log,puerto_interrupt);

  if (socket_servidor_Dispatch == -1 || socket_servidor_Interrupt == -1) {
        log_error(log, "Error al iniciar el servidor de CPU");
        // Cerrar los sockets abiertos
        if (socket_servidor_Dispatch != -1) close(socket_servidor_Dispatch);
        if (socket_servidor_Interrupt != -1) close(socket_servidor_Interrupt);
        free(sockets);
        return NULL;
    }

    log_info(log,"Servidor abierto correctamente");

socket_cliente_Dispatch = esperar_cliente(log,socket_servidor_Dispatch);
socket_cliente_Interrupt = esperar_cliente(log,socket_servidor_Interrupt);

if (socket_cliente_Dispatch == -1 || socket_cliente_Interrupt == -1) {
        log_error(log, "Error al esperar cliente");
        if (socket_cliente_Dispatch != -1) close(socket_cliente_Dispatch);
        if (socket_cliente_Interrupt != -1) close(socket_cliente_Interrupt);
        close(socket_servidor_Dispatch);
        close(socket_servidor_Interrupt);
        free(sockets);
        return NULL;
    }
printf("hola");
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

sockets->socket_Dispatch=socket_servidor_Dispatch;
sockets->socket_Interrupt=socket_servidor_Interrupt;

    close(socket_cliente_Dispatch);
    close(socket_cliente_Interrupt);
	return sockets;
}

int cliente_cpu_memoria (t_log* log, t_config * config){


char * ip, * puerto;
int socket_cliente, respuesta;

ip = config_get_string_value(config, "IP_MEMORIA");
puerto = config_get_string_value(config, "PUERTO_MEMORIA");

 // Verificar que ip y puerto no sean NULL
    if (ip == NULL || puerto == NULL) {
        log_info(log, "No se pudo obtener IP o PUERTO de la configuración");
        return -1;
    }

    // Crear conexion
    socket_cliente = crear_conexion(log, ip, puerto);

    if (socket_cliente == -1) {
        log_info(log, "No se pudo crear la conexion");
        return -1;
    }

   respuesta = cliente_handshake(socket_cliente,log);
   

   if (respuesta == 0){
    log_info(log,"Handshake de CPU --> Memoria realizado correctamente");
   }
   else {
    log_error(log, "Handshake de CPU --> Memoria tuvo un error");
   }



    return socket_cliente;
}

void* funcion_hilo_servidor_cpu(void* void_args){

  args_hilo* args = ((args_hilo*)void_args);


    t_socket_cpu* sockets = servidor_CPU_Kernel(args->log, args->config);
   
    if (sockets->socket_Dispatch == -1 || sockets->socket_Interrupt == -1) {
        log_error(args->log, "No se pudo establecer la conexion con Kernel");
        pthread_exit(NULL);
    }
    
    return (void*)sockets;
}
void* funcion_hilo_cliente_memoria(void* void_args){

  args_hilo* args = ((args_hilo*)void_args);


    int socket = cliente_cpu_memoria(args->log, args->config);
    if (socket== -1) {
        log_error(args->log, "No se pudo establecer la conexion con Kernel");
        pthread_exit(NULL);
    }
    return (void*)(intptr_t)socket;
}


t_sockets_cpu* hilos_cpu(t_log* log, t_config* config){

pthread_t hilo_servidor;
pthread_t hilo_cliente;

args_hilo* args = malloc(sizeof(args_hilo)); 

t_sockets_cpu* sockets= malloc(sizeof(t_sockets_cpu));

args->config=config;
args->log=log;

void* socket_servidor_kernel;
void* socket_cliente_memoria;

int resultado;

resultado = pthread_create (&hilo_servidor,NULL,funcion_hilo_servidor_cpu,(void*)args);

if(resultado != 0){
    log_error(log,"Error al crear el hilo");
    free (args);
    return NULL;
}

log_info(log,"El hilo servidor_kernel se creo correctamente");

resultado = pthread_create (&hilo_cliente,NULL,funcion_hilo_cliente_memoria,(void*)args);

if(resultado != 0){
    log_error(log,"Error al crear el hilo");
    free (args);
    return NULL;
}

log_info(log,"El hilo cliente_memoria se creo correctamente");

pthread_join(hilo_servidor,&socket_servidor_kernel);

sockets->socket_servidor= (t_socket_cpu*)socket_servidor_kernel;

pthread_join(hilo_cliente,&socket_cliente_memoria);

sockets->socket_cliente= (intptr_t)socket_cliente_memoria;

free(args);
return sockets;
}