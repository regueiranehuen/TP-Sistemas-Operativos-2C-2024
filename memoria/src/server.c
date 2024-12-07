#include "includes/server.h"

t_sockets *sockets_iniciales;
static pthread_mutex_t cliente_count_mutex = PTHREAD_MUTEX_INITIALIZER;
static int client_count = 0; // numero incremental del numero del cliente
sem_t sem_conexion_hecha;
sem_t sem_fin_memoria;
sem_t sem_conexion_iniciales;
sem_t sem_termina_hilo;

pthread_mutex_t mutex_estado_memoria;
pthread_mutex_t mutex_lista_particiones;
pthread_mutex_t mutex_logs;

t_list *lista_contextos_pids;
t_list *lista_instrucciones_tid_pid;

pthread_mutex_t mutex_lista_contextos_pids;

void *hilo_por_cliente(void *void_args)
{

    hilo_clientes *args = (hilo_clientes *)void_args;

    int socket_cliente = esperar_cliente(args->log, args->socket_servidor);
    if (socket_cliente == -1)
    {
        pthread_mutex_lock(&mutex_logs);
        log_error(args->log, "Error al esperar cliente");
        pthread_mutex_unlock(&mutex_logs);
        free(args);
        return NULL;
    }

    int cliente_n;
    pthread_mutex_lock(&cliente_count_mutex);
    cliente_n = client_count + 1;
    client_count++;
    pthread_mutex_unlock(&cliente_count_mutex);

    
    int resultado = servidor_handshake(socket_cliente, args->log);
    

    if (resultado == 0)
    {
        pthread_mutex_lock(&mutex_logs);
        log_info(args->log, "Handshake memoria -> cliente_%d realizado correctamente", cliente_n);
        pthread_mutex_unlock(&mutex_logs);
    }
    if (cliente_n <= 2)
    { // conexiones iniciales de cpu y kernel

        code_operacion modulo = recibir_code_operacion(socket_cliente);
        switch (modulo)
        {
        case CPU:
            sockets_iniciales->socket_cpu = socket_cliente;
            pthread_mutex_lock(&mutex_logs);
            log_info(logger, "1_socket de cpu:%d", sockets_iniciales->socket_cpu);
            pthread_mutex_unlock(&mutex_logs);
            sem_post(&sem_conexion_hecha);
            sem_post(&sem_conexion_iniciales);
            break;
        case KERNEL:
            sockets_iniciales->socket_kernel = socket_cliente;
            pthread_mutex_lock(&mutex_logs);
            log_info(logger, "1_socket de kernel:%d", sockets_iniciales->socket_kernel);
            pthread_mutex_unlock(&mutex_logs);
            sem_post(&sem_conexion_hecha);
            sem_post(&sem_conexion_iniciales);
            break;
        default:
            pthread_mutex_lock(&mutex_logs);
            log_info(logger, "Llego este codigo de operacion: %d", modulo);
            pthread_mutex_unlock(&mutex_logs);
            break;
        }
    }
    else
    {
        pthread_mutex_lock(&mutex_logs);
        log_debug(args->log, "## Kernel Conectado - FD del socket: %d", socket_cliente);
        pthread_mutex_unlock(&mutex_logs);
        sem_post(&sem_conexion_hecha);
        atender_conexiones(socket_cliente);
    }

    free(args);

    return NULL;
}

void *gestor_clientes(void *void_args)
{ // Crear un hilo que crea hilos que crean conexiones para cada petición de kernel

    hilo_clientes *args = (hilo_clientes *)void_args;

    int respuesta;
    int i = 0;
    while (1)
    { // mientras el servidor este abierto
        
        hilo_clientes *args_hilo = malloc(sizeof(hilo_clientes));
        args_hilo->log = args->log;
        args_hilo->socket_servidor = args->socket_servidor;

        pthread_t hilo_cliente;
        
        respuesta = pthread_create(&hilo_cliente, NULL, hilo_por_cliente, (void *)args_hilo);
        
        if (respuesta != 0)
        {
            pthread_mutex_lock(&mutex_logs);
            log_error(args->log, "Error al crear el hilo para el cliente");
            pthread_mutex_unlock(&mutex_logs);
            free(args_hilo);
            continue;
        }
        pthread_detach(hilo_cliente);
        sem_wait(&sem_conexion_hecha); // esperar a que un cliente se conecte para esperar otro
        pthread_mutex_lock(&mutex_estado_memoria);
        if (estado_memoria == 0){
            free(args);
            sem_post(&sem_termina_hilo);
            pthread_mutex_unlock(&mutex_estado_memoria);
            return NULL;
        }
        pthread_mutex_unlock(&mutex_estado_memoria);
        

        i++;
    }
    return NULL;
}

int servidor_memoria(t_log *log, t_config *config)
{

    hilo_clientes *args = malloc(sizeof(hilo_clientes));

    char *puerto;
    int socket_servidor, respuesta;

    pthread_t hilo_gestor;

    puerto = config_get_string_value(config, "PUERTO_ESCUCHA");

    socket_servidor = iniciar_servidor(log, puerto);

    args->log = log;
    args->socket_servidor = socket_servidor;

    if (socket_servidor == -1)
    {
        pthread_mutex_lock(&mutex_logs);
        log_error(log, "Error al iniciar el servidor");
        pthread_mutex_unlock(&mutex_logs);
        return -1;
    }

    pthread_mutex_lock(&mutex_logs);
    log_info(log, "Servidor abierto correctamente");
    pthread_mutex_unlock(&mutex_logs);

    respuesta = pthread_create(&hilo_gestor, NULL, gestor_clientes, args);

    if (respuesta != 0)
    {
        pthread_mutex_lock(&mutex_logs);
        log_error(log, "Error al crear el hilo_gestor_clientes");
        pthread_mutex_unlock(&mutex_logs);
        free(args);
        return -1;
    }

    pthread_detach(hilo_gestor);
    
    return socket_servidor;
}

void *funcion_hilo_servidor(void *void_args)
{

    args_hilo *args = (args_hilo *)void_args;

    int socket_servidor = servidor_memoria(args->log, args->config);
    if (socket_servidor == -1)
    {
        pthread_mutex_lock(&mutex_logs);
        log_error(args->log, "No se pudo establecer la conexion con la Memoria");
        pthread_mutex_unlock(&mutex_logs);
        return NULL;
    }

    return (void *)(intptr_t)socket_servidor;
}

int cliente_memoria_filesystem(t_log *log, t_config *config)
{

    char *ip, *puerto;
    int socket_cliente, respuesta;

    ip = config_get_string_value(config, "IP_FILESYSTEM");
    puerto = config_get_string_value(config, "PUERTO_FILESYSTEM");

    // Verificar que ip y puerto no sean NULL
    if (ip == NULL || puerto == NULL)
    {
        pthread_mutex_lock(&mutex_logs);
        log_info(log, "No se pudo obtener IP o PUERTO de la configuración");
        pthread_mutex_unlock(&mutex_logs);
        return -1;
    }

    // Crear conexion
    socket_cliente = crear_conexion(log, ip, puerto);

    if (socket_cliente == -1)
    {
        pthread_mutex_lock(&mutex_logs);
        log_info(log, "No se pudo crear la conexion");
        pthread_mutex_unlock(&mutex_logs);
        return -1;
    }

    respuesta = cliente_handshake(socket_cliente, log);

    if (respuesta == 0)
    {
        pthread_mutex_lock(&mutex_logs);
        log_info(log, "Handshake de Memoria --> Filesystem realizado correctamente");
        pthread_mutex_unlock(&mutex_logs);
    }
    else
    {
        pthread_mutex_lock(&mutex_logs);
        log_error(log, "Handshake de Memoria --> Filesystem tuvo un error");
        pthread_mutex_unlock(&mutex_logs);

    }

    return socket_cliente;
}

void *funcion_hilo_cliente(void *void_args)
{

    args_hilo *args = ((args_hilo *)void_args);

    int socket_cliente = cliente_memoria_filesystem(args->log, args->config);
    if (socket_cliente == -1)
    {
        pthread_mutex_lock(&mutex_logs);
        log_error(args->log, "No se pudo establecer la conexion con filesystem");
        pthread_mutex_unlock(&mutex_logs);
        pthread_exit(NULL);
    }

    return (void *)(intptr_t)socket_cliente;
}

sockets_memoria *hilos_memoria(t_log *log, t_config *config)
{

    pthread_t hilo_servidor;
    pthread_t hilo_cliente;

    args_hilo *args = malloc(sizeof(args_hilo));

    args->config = config;
    args->log = log;

    void *socket_cliente;
    void *socket_servidor;

    int resultado;

    sockets_memoria *sockets = malloc(sizeof(sockets_memoria));

    resultado = pthread_create(&hilo_cliente, NULL, funcion_hilo_cliente, args);

    if (resultado != 0)
    {
        pthread_mutex_lock(&mutex_logs);
        log_error(log, "Error al crear el hilo");
        pthread_mutex_unlock(&mutex_logs);
        free(args);
        return NULL;
    }

    pthread_mutex_lock(&mutex_logs);
    log_info(log, "El hilo cliente se creo correctamente");
    pthread_mutex_unlock(&mutex_logs);

    resultado = pthread_create(&hilo_servidor, NULL, funcion_hilo_servidor, args);

    if (resultado != 0)
    {
        pthread_mutex_lock(&mutex_logs);
        log_error(log, "Error al crear el hilo");
        pthread_mutex_unlock(&mutex_logs);
        free(args);
        return NULL;
    }

    pthread_mutex_lock(&mutex_logs);
    log_info(log, "El hilo servidor se creo correctamente");
    pthread_mutex_unlock(&mutex_logs);

    pthread_join(hilo_cliente, &socket_cliente);
    pthread_join(hilo_servidor, &socket_servidor);

    resultado = (intptr_t)socket_cliente;

    sockets->socket_cliente = resultado;

    resultado = (intptr_t)socket_servidor;

    sockets->socket_servidor = resultado;

    free(args);
    return sockets;
}