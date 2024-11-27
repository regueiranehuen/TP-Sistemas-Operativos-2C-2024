#include "includes/server.h"
#include "includes/peticiones.h"

int estado_filesystem;
sem_t sem_conexion_hecha;
static int client_count = 0;
static pthread_mutex_t cliente_count_mutex = PTHREAD_MUTEX_INITIALIZER;

void *hilo_por_cliente(void *void_args){

    hilo_clientes *args = (hilo_clientes *)void_args;

    int socket_cliente = esperar_cliente(args->log, args->socket_servidor);
    if (socket_cliente == -1)
    {
        log_error(args->log, "Error al esperar cliente");
        free(args);
        return NULL;
    }

    int cliente_n;
    pthread_mutex_lock(&cliente_count_mutex);
    cliente_n = client_count + 1;
    client_count++;
    pthread_mutex_unlock(&cliente_count_mutex);

    
    int resultado = servidor_handshake(socket_cliente, args->log);
    

    if (resultado == 0){
        log_info(args->log, "Handshake filesystem -> cliente_%d realizado correctamente", cliente_n);
    }

    if (cliente_n < 1){ // conexion inicial con memoria
        log_info(args->log, "HOLAAA");
        sem_post(&sem_conexion_hecha);
        close(socket_cliente);
    }

    else{

        log_info(args->log, "%d_Peticion de Memoria", socket_cliente);
        sem_post(&sem_conexion_hecha);
        atender_conexiones(socket_cliente);
    }

    free(args);

    return NULL;
}

void *gestor_clientes(void *void_args){ // Crear un hilo que crea hilos que crean conexiones para cada petición de kernel

    hilo_clientes *args = (hilo_clientes *)void_args;

    int respuesta;
    int i = 0;
    while (estado_filesystem != 0)
    { // mientras el servidor este abierto
        
        hilo_clientes *args_hilo = malloc(sizeof(hilo_clientes));
        args_hilo->log = args->log;
        args_hilo->socket_servidor = args->socket_servidor;

        pthread_t hilo_cliente;
        
        respuesta = pthread_create(&hilo_cliente, NULL, hilo_por_cliente, (void *)args_hilo);
        
        if (respuesta != 0)
        {
            log_error(args->log, "Error al crear el hilo para el cliente");
            free(args_hilo);
            continue;
        }
        pthread_detach(hilo_cliente);
        sem_wait(&sem_conexion_hecha); // esperar a que un cliente se conecte para esperar otro
        i++;
    }
    return NULL;
}

int servidor_FileSystem_Memoria(t_log* log, t_config* config){

    hilo_clientes *args = malloc(sizeof(hilo_clientes));

    pthread_t hilo_gestor;

    char * puerto;
    int socket_servidor, respuesta;

    puerto = config_get_string_value(config,"PUERTO_ESCUCHA");

    socket_servidor = iniciar_servidor (log,puerto);

    if(socket_servidor ==-1){
        log_error(log, "Error al iniciar el servidor de Filesystem");
        return socket_servidor;
    }

    log_info(log,"Servidor abierto correctamente");

    args->log = log;
    args->socket_servidor = socket_servidor;

    respuesta = pthread_create(&hilo_gestor, NULL, gestor_clientes, args);

        if (respuesta != 0){
            log_error(log, "Error al crear el hilo_gestor_clientes");
            free(args);
            return -1;
        }

        pthread_detach(hilo_gestor);
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
