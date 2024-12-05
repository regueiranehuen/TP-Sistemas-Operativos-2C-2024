#include "includes/server.h"
#include "includes/peticiones.h"

int estado_filesystem;
sem_t sem_conexion_hecha;
sem_t sem_termina_hilo;
static int client_count = 0;
pthread_mutex_t cliente_count_mutex;
pthread_mutex_t mutex_logs;

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
        pthread_mutex_lock(&mutex_logs);
        log_info(args->log, "Handshake filesystem -> cliente_%d realizado correctamente", cliente_n);
        pthread_mutex_unlock(&mutex_logs);
    }

    if (cliente_n < 1){ // conexion inicial con memoria
        sem_post(&sem_conexion_hecha);
        close(socket_cliente);
    }

    else{
        pthread_mutex_lock(&mutex_logs);
        log_info(args->log, "%d_Peticion de Memoria", socket_cliente);
        pthread_mutex_unlock(&mutex_logs);
        sem_post(&sem_conexion_hecha);
        atender_conexiones(socket_cliente);

    }

    free(args);

    return NULL;
}

void *gestor_clientes(void *void_args) {
    hilo_clientes *args = (hilo_clientes *)void_args;

    int respuesta;
    int i = 0;

    while (1) { // Ciclo infinito hasta que estado_filesystem sea 0
        pthread_mutex_lock(&mutex_estado_filesystem);
        int estado = estado_filesystem;
        pthread_mutex_unlock(&mutex_estado_filesystem);

        if (estado == 0) {
            break;
        }

        hilo_clientes *args_hilo = malloc(sizeof(hilo_clientes));
        if (args_hilo == NULL) {
            log_error(args->log, "Error al asignar memoria para el hilo del cliente");
            continue;
        }

        args_hilo->log = args->log;
        args_hilo->socket_servidor = args->socket_servidor;

        pthread_t hilo_cliente;

        respuesta = pthread_create(&hilo_cliente, NULL, hilo_por_cliente, (void *)args_hilo);

        if (respuesta != 0) {
            log_error(args->log, "Error al crear el hilo para el cliente");
            free(args_hilo);
            continue;
        }

        pthread_detach(hilo_cliente);
        
        sem_wait(&sem_conexion_hecha); // Esperar a que un cliente se conecte para procesar otro

        pthread_mutex_lock(&mutex_estado_filesystem);
        if (estado_filesystem == 0) {
            pthread_mutex_unlock(&mutex_estado_filesystem);
            free(args);
            sem_post(&sem_termina_hilo);
            return NULL;
        }
        pthread_mutex_unlock(&mutex_estado_filesystem);

        i++;
    }

    free(args);
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
        pthread_mutex_lock(&mutex_logs);
        log_error(log, "Error al iniciar el servidor de Filesystem");
        pthread_mutex_unlock(&mutex_logs);
        return socket_servidor;
    }
    pthread_mutex_lock(&mutex_logs);
    log_info(log,"Servidor abierto correctamente");
    pthread_mutex_unlock(&mutex_logs);
    args->log = log;
    args->socket_servidor = socket_servidor;

    respuesta = pthread_create(&hilo_gestor, NULL, gestor_clientes, args);

        if (respuesta != 0){
            pthread_mutex_lock(&mutex_logs);
            log_error(log, "Error al crear el hilo_gestor_clientes");
            pthread_mutex_unlock(&mutex_logs);
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
        pthread_mutex_lock(&mutex_logs);
        log_error(args->log, "No se pudo establecer la conexion con Memoria");
        pthread_mutex_unlock(&mutex_logs);
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
        pthread_mutex_lock(&mutex_logs);
        log_error(log,"Error al crear el hilo");
        pthread_mutex_unlock(&mutex_logs);
        free (args);
        return -1;
    }
    pthread_mutex_lock(&mutex_logs);
    log_info(log,"El hilo servidor se creo correctamente");
    pthread_mutex_unlock(&mutex_logs);
    pthread_join(hilo_servidor,&socket_servidor);

    resultado = (intptr_t)socket_servidor;

    free(args);
    return resultado;
}