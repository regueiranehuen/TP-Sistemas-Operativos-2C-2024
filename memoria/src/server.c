#include "server.h"

void leer_config(char* path_config){
    config= iniciar_config(path_config);

    puerto_escucha = config_get_int_value(config,"PUERTO_ESCUCHA");
    ip_file = config_get_string_value(config,"IP_FILESYSTEM");
    puerto_file = config_get_int_value(config,"PUERTO_FILESYSTEM");
    tam_memoria = config_get_int_value(config,"TAM_MEMORIA");
    path_instrucciones = config_get_string_value(config,"PATH_INSTRUCCIONES");
    retardo_respuesta = config_get_int_value(config,"RETARDO_RESPUESTA");
    esquema = config_get_string_value(config,"ESQUEMA");
    algoritmo = config_get_string_value(config,"ALGORITMO_BUSQUEDA");
    particiones = config_get_array_value(config,"PARTICIONES");
    log_level = config_get_string_value(config,"LOG_LEVEL");

}

void* hilo_gestor_clientes (void* void_args){
    hilo_clientes *args = (hilo_clientes*)void_args;

    int socket_cliente = esperar_cliente(args->log,args->socket_servidor);
    if (socket_cliente == -1) {
        log_error(args->log, "Error al esperar cliente");
        close(args->socket_servidor);
    }

    servidor_handshake(socket_cliente,args->log);
    log_info(args->log,"Handshake memoria->cliente realizado correctamente");
    close(socket_cliente);
    return NULL;
}

int servidor_memoria_kernel (t_log* log, t_config* config){
    int respuesta;
    hilo_clientes* args1 = malloc(sizeof(hilo_clientes)); 
    hilo_clientes* args2 = malloc(sizeof(hilo_clientes)); 

    socket_servidor = iniciar_servidor(log,puerto_escucha);

    args1->log=log;
    args1->socket_servidor=socket_servidor;
    args2->log=log;
    args2->socket_servidor=socket_servidor;

    if (socket_servidor == -1) {
        log_error(log, "Error al iniciar el servidor");
        return -1;
        }

        log_info(log,"Servidor abierto correctamente");

    respuesta = pthread_create (&hilo_socket_1,NULL,hilo_gestor_clientes,(void*)args1);

    if(respuesta != 0){
        log_error(log,"Error al crear el hilo_gestor_cliente_1");
        free (args1);
        return -1;
    }

    log_info(log,"El hilo_gestor_cliente_1 se creo correctamente");

    respuesta = pthread_create (&hilo_socket_2,NULL,hilo_gestor_clientes,(void*)args2);

    if(respuesta != 0){
        log_error(log,"Error al crear el hilo_gestor_cliente_1");
        free (args2);
        return -1;
    }

    log_info(log,"El hilo_gestor_cliente_2 se creo correctamente");

    pthread_join(hilo_socket_1,NULL);
    pthread_join(hilo_socket_2,NULL);

        free(args1);
        free(args2);
        return socket_servidor;
}

void* funcion_hilo_servidor(void *void_args){
    
    args_hilo *args = (args_hilo*)void_args;

    int socket_servidor = servidor_memoria_kernel(args->log,args->config);
    if (socket_servidor == -1) {
        log_error(args->log, "No se pudo establecer la conexion con la Memoria");
        pthread_exit(NULL);
    }

   return (void*)(intptr_t)socket_servidor;
}

int cliente_memoria_filesystem (t_log* log, t_config* config){
 // Verificar que ip y puerto_escucha no sean NULL
    int respuesta;
    if (ip_file == NULL || puerto_file == NULL) {
        log_info(log, "No se pudo obtener IP o PUERTO de la configuración");
        return -1;
    }

    // Crear conexion
    socket_cliente = crear_conexion(log, ip_file, puerto_file);

    if (socket_cliente == -1) {
        log_info(log, "No se pudo crear la conexion");
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

void* funcion_hilo_cliente(void* void_args){
    args_hilo* args = ((args_hilo*)void_args);

    int socket_cliente = cliente_memoria_filesystem(args->log,args->config);
    if (socket_cliente == -1) {
        log_error(args->log, "No se pudo establecer la conexion con filesystem");
        pthread_exit(NULL);
    }

   return (void*)(intptr_t)socket_cliente;
}

sockets_memoria* hilos_memoria(t_log* log, t_config* config){
    int resultado;
    args_hilo* args = malloc(sizeof(args_hilo)); 

    args->config=config;
    args->log=log;

    sockets_memoria* sockets=malloc(sizeof(sockets_memoria));


    resultado = pthread_create (&hilo_servidor,NULL,funcion_hilo_servidor,(void*)args);

    if(resultado != 0){
        log_error(log,"Error al crear el hilo");
        free (args);
        return NULL;
    }

    log_info(log,"El hilo servidor se creo correctamente");

    resultado = pthread_create (&hilo_cliente,NULL,funcion_hilo_cliente,(void*)args);

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
