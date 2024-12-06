#include "includes/cliente.h"
#include "includes/procesos.h"

pthread_mutex_t mutex_i;

int cliente_Memoria_Kernel(t_log* log, t_config* config) {
    char* ip;
    char* puerto;
    int socket_cliente;
    static int i=0;

    // Asignar valores a las variables ip y puerto usando config_get_string_value
    ip = config_get_string_value(config, "IP_MEMORIA");
    puerto = config_get_string_value(config, "PUERTO_MEMORIA");

    // Verificar que ip y puerto no sean NULL
    if (ip == NULL || puerto == NULL) {
        pthread_mutex_lock(&mutex_log);
        log_info(log, "No se pudo obtener IP o PUERTO de la configuración");
        pthread_mutex_unlock(&mutex_log);
        return -1;
    }

    // Crear conexion
    socket_cliente = crear_conexion(log, ip, puerto);
    if (socket_cliente == -1) {
        pthread_mutex_lock(&mutex_log);
        log_info(log, "No se pudo crear la conexion");
        pthread_mutex_unlock(&mutex_log);
        return -1;
    }

    int respuesta = cliente_handshake(socket_cliente,log);

    if (respuesta == 0){
    pthread_mutex_lock(&mutex_i);
    pthread_mutex_lock(&mutex_log);
    log_info(log,"%d_Handshake de Kernel --> Memoria realizado correctamente",i);
    pthread_mutex_unlock(&mutex_log);
    pthread_mutex_unlock(&mutex_i);
   }
   else {
    pthread_mutex_lock(&mutex_i);
    pthread_mutex_lock(&mutex_log);
    log_error(log, "%d_Handshake de Kernel --> Memoria tuvo un error",i);
    pthread_mutex_unlock(&mutex_log);
    pthread_mutex_unlock(&mutex_i);
   }
   pthread_mutex_lock(&mutex_i);
   if(i==0){//conexion inicial
   
    code_operacion cod_op = KERNEL;
    send_code_operacion(cod_op,socket_cliente);
   }

   i++;
   pthread_mutex_unlock(&mutex_i);
   return socket_cliente;
}

t_socket_cpu* cliente_CPU_Kernel(t_log* log, t_config* config){

    char* ip;
    char* puerto_Dispatch, * puerto_Interrupt;
    int socket_cliente_Interrupt,socket_cliente_Dispatch, respuesta_Dispatch, respuesta_Interrupt;
    t_socket_cpu* resultado=malloc(sizeof(t_socket_cpu));
    resultado->socket_Dispatch = -1;
    resultado->socket_Interrupt = -1;

    // Asignar valores a las variables ip y puerto usando config_get_string_value
    ip = config_get_string_value(config, "IP_CPU");
    puerto_Dispatch = config_get_string_value(config, "PUERTO_CPU_DISPATCH");
    puerto_Interrupt= config_get_string_value(config, "PUERTO_CPU_INTERRUPT"); 

    // Verificar que ip y puerto no sean NULL
    if (ip == NULL || puerto_Dispatch == NULL || puerto_Interrupt == NULL) {
        pthread_mutex_lock(&mutex_log);
        log_info(log, "No se pudo obtener IP o PUERTO de la configuracion");
        pthread_mutex_unlock(&mutex_log);
        return resultado;
    }

    // Crear conexion a Interrupt
  
    socket_cliente_Interrupt = crear_conexion(log, ip, puerto_Interrupt);
  
    if (socket_cliente_Interrupt == -1) {
        pthread_mutex_lock(&mutex_log);
        log_error(log, "No se pudo crear la conexion a CPU_Interrupt");
        pthread_mutex_unlock(&mutex_log);
    } else {
        pthread_mutex_lock(&mutex_log);
        log_info(log, "Se pudo conectar al servidor CPU_Interrupt");
        pthread_mutex_unlock(&mutex_log);
    }

    // Crear conexion a Dispatch
    socket_cliente_Dispatch = crear_conexion(log, ip, puerto_Dispatch);
    if (socket_cliente_Dispatch == -1) {
        pthread_mutex_lock(&mutex_log);
        log_error(log, "No se pudo crear la conexion a CPU_Dispatch");
        pthread_mutex_unlock(&mutex_log);
    } else {
        pthread_mutex_lock(&mutex_log);
        log_info(log, "Se pudo conectar al servidor CPU_Dispatch");
        pthread_mutex_unlock(&mutex_log);
    }

   respuesta_Dispatch = cliente_handshake(socket_cliente_Dispatch,log);
   respuesta_Interrupt = cliente_handshake(socket_cliente_Interrupt,log);
   
   if (respuesta_Interrupt == 0){
    pthread_mutex_lock(&mutex_log);
    log_info(log,"Handshake de Kernel --> CPU_Interrupt realizado correctamente");
    pthread_mutex_unlock(&mutex_log);
   }
   else {
    pthread_mutex_lock(&mutex_log);
    log_error(log, "Handshake de Kernel --> CPU_Interrupt tuvo un error");
    pthread_mutex_unlock(&mutex_log);
   }
   if (respuesta_Dispatch == 0){
    pthread_mutex_lock(&mutex_log);
    log_info(log,"Handshake de Kernel --> CPU_Dispatch realizado correctamente");
    pthread_mutex_unlock(&mutex_log);
   }
   else {
    pthread_mutex_lock(&mutex_log);
    log_error(log, "Handshake de Kernel --> CPU_Dispatch tuvo un error");
    pthread_mutex_unlock(&mutex_log);
   }

   resultado->socket_Dispatch=socket_cliente_Dispatch;
   resultado->socket_Interrupt=socket_cliente_Interrupt;

    return resultado;
}

void* funcion_hilo_cliente_memoria(void* void_args){
    
    args_hilo* args = ((args_hilo*)void_args);


    int socket_cliente = cliente_Memoria_Kernel(args->log, args->config);
    if (socket_cliente == -1) {
        pthread_mutex_lock(&mutex_log);
        log_error(args->log, "No se pudo establecer la conexion con Memoria");
        pthread_mutex_unlock(&mutex_log);
        pthread_exit(NULL);
    }

   return (void*)(intptr_t)socket_cliente;
}

void* funcion_hilo_cliente_cpu(void* void_args){

  args_hilo* args = ((args_hilo*)void_args);

    t_socket_cpu* sockets = cliente_CPU_Kernel(args->log, args->config);
    if (sockets->socket_Dispatch == -1 || sockets->socket_Interrupt == -1) {
        pthread_mutex_lock(&mutex_log);
        log_error(args->log, "No se pudo establecer la conexion con CPU");
        pthread_mutex_unlock(&mutex_log);
        pthread_exit(NULL);
    }
    
    return (void*)sockets;
}


sockets_kernel* hilos_kernel(t_log* log, t_config* config){

pthread_t hilo_cliente_memoria;
pthread_t hilo_cliente_cpu;

args_hilo* args = malloc(sizeof(args_hilo)); 

sockets_kernel* sockets= malloc(sizeof(sockets_kernel));

args->config=config;
args->log=log;

void* socket_cliente_memoria;
void* socket_cliente_cpu;

int resultado;

resultado = pthread_create (&hilo_cliente_memoria,NULL,funcion_hilo_cliente_memoria,(void*)args);

if(resultado != 0){
    pthread_mutex_lock(&mutex_log);
    log_error(log,"Error al crear el hilo");
    pthread_mutex_unlock(&mutex_log);
    free (args);
    return NULL;
}
pthread_mutex_lock(&mutex_log);
log_info(log,"El hilo cliente_memoria se creo correctamente");
pthread_mutex_unlock(&mutex_log);

resultado = pthread_create (&hilo_cliente_cpu,NULL,funcion_hilo_cliente_cpu,(void*)args);

if(resultado != 0){
    pthread_mutex_lock(&mutex_log);
    log_error(log,"Error al crear el hilo");
    pthread_mutex_unlock(&mutex_log);
    free (args);
    return NULL;
}

pthread_mutex_lock(&mutex_log);
log_info(log,"El hilo cliente_cpu se creo correctamente");
pthread_mutex_unlock(&mutex_log);

pthread_join(hilo_cliente_memoria,&socket_cliente_memoria);

sockets->socket_cliente_memoria= (intptr_t)socket_cliente_memoria;

pthread_join(hilo_cliente_cpu,&socket_cliente_cpu);

sockets->sockets_cliente_cpu= (void*)socket_cliente_cpu;

free(args);
return sockets;
}