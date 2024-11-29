#include "server.h"
#include "cicloDeInstruccion.h"

t_log *log_cpu = NULL;
t_config *config = NULL;
t_sockets_cpu *sockets_cpu = NULL;

char *ip_memoria = NULL;
int puerto_memoria = 0;
int puerto_escucha_dispatch = 0;
int puerto_escucha_interrupt = 0;
char* log_level_config = NULL;

int socket_servidor_Dispatch = 0, socket_servidor_Interrupt = 0;
int socket_cliente_Dispatch = 0, socket_cliente_Interrupt = 0;
int respuesta_Dispatch = 0, respuesta_Interrupt = 0;

t_socket_cpu *sockets = NULL;

pthread_t hilo_servidor;
pthread_t hilo_cliente;
void *socket_servidor_kernel = NULL;
void *socket_cliente_memoria = NULL;

uint32_t tid_interrupt;
bool hay_interrupcion = false;
int es_por_usuario = 0;



pthread_mutex_t mutex_contextos_exec;
pthread_mutex_t mutex_interrupt;


t_contexto_tid*contexto_tid_actual;
t_contexto_pid*contexto_pid_actual;

sem_t sem_ciclo_instruccion;
sem_t sem_ok_o_interrupcion;
sem_t sem_finalizacion_cpu;
sem_t sem_socket_cerrado;



code_operacion devolucion_kernel;



// Lectura de configuración
void leer_config(char *path)
{
    config = iniciar_config(path);
    ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    puerto_memoria = config_get_int_value(config, "PUERTO_MEMORIA");
    puerto_escucha_dispatch = config_get_int_value(config, "PUERTO_ESCUCHA_DISPATCH");
    puerto_escucha_interrupt = config_get_int_value(config, "PUERTO_ESCUCHA_INTERRUPT");
    log_level_config = config_get_string_value(config, "LOG_LEVEL");
    log_info(log_cpu, "Configuración del CPU cargada.");
}

// Inicialización del servidor de CPU para el Kernel
t_socket_cpu *servidor_CPU_Kernel(t_log *log, t_config *config)
{
    t_socket_cpu *sockets = malloc(sizeof(t_socket_cpu));
    if (!sockets)
    {
        log_error(log, "Error al asignar memoria para sockets.");
        return NULL;
    }

    char puerto_str[6];
    sprintf(puerto_str, "%d", puerto_escucha_dispatch);
    socket_servidor_Dispatch = iniciar_servidor(log, puerto_str);

    sprintf(puerto_str, "%d", puerto_escucha_interrupt);
    socket_servidor_Interrupt = iniciar_servidor(log, puerto_str);

    if (socket_servidor_Dispatch == -1 || socket_servidor_Interrupt == -1)
    {
        log_error(log, "Error al iniciar servidores.");
        if (socket_servidor_Dispatch != -1)
            close(socket_servidor_Dispatch);
        if (socket_servidor_Interrupt != -1)
            close(socket_servidor_Interrupt);
        free(sockets);
        return NULL;
    }

    socket_cliente_Dispatch = esperar_cliente(log, socket_servidor_Dispatch);
    socket_cliente_Interrupt = esperar_cliente(log, socket_servidor_Interrupt);

    if (socket_cliente_Dispatch == -1 || socket_cliente_Interrupt == -1)
    {
        log_error(log, "Error al esperar cliente.");
        close(socket_servidor_Dispatch);
        close(socket_servidor_Interrupt);
        free(sockets);
        return NULL;
    }

    respuesta_Dispatch = servidor_handshake(socket_cliente_Dispatch, log);
    respuesta_Interrupt = servidor_handshake(socket_cliente_Interrupt, log);

    if (respuesta_Dispatch == 0){
    log_info(log,"Socket Dispatch: %d", socket_cliente_Dispatch);
    log_info(log,"Handshake de CPU_Dispatch --> Kernel realizado correctamente");
    }
   else {
    log_error(log, "Handshake de CPU_Dispatch --> Kernel tuvo un error");
   }
   if (respuesta_Interrupt == 0){
    log_info(log,"Socket Dispatch: %d", socket_cliente_Interrupt);
    log_info(log,"Handshake de CPU_Interrupt --> Kernel realizado correctamente");
   }
   else {
    log_error(log, "Handshake de CPU_Interrupt --> Kernel tuvo un error");
   }

    sockets->socket_servidor_Dispatch = socket_servidor_Dispatch;
    sockets->socket_servidor_Interrupt = socket_servidor_Interrupt;
    sockets->socket_cliente_Dispatch = socket_cliente_Dispatch;
    sockets->socket_cliente_Interrupt = socket_cliente_Interrupt;

    return sockets;
}

// Conexión cliente CPU-Memoria
int cliente_cpu_memoria(t_log *log, t_config *config)
{
    if (!ip_memoria)
    {
        log_error(log, "IP de memoria no definida en la configuración.");
        return -1;
    }

    char puerto_str[6];
    sprintf(puerto_str, "%d", puerto_memoria); // Línea 64
    int socket_cliente = crear_conexion(log, ip_memoria, puerto_str);
    if (socket_cliente == -1)
    {
        log_error(log, "Error al crear la conexión con memoria.");
        return -1;
    }

    int respuesta = cliente_handshake(socket_cliente, log);
    if (respuesta != 0)
    {
        log_error(log, "Error en el handshake con memoria.");
        close(socket_cliente);
        return -1;
    }

    if (respuesta == 0){
    log_info(log,"Socket Memoria: %d", socket_cliente);
    log_info(log,"Handshake de CPU --> Memoria realizado correctamente");
    }
   else {
    log_error(log, "Handshake de CPU --> Memoria tuvo un error");
   }

    code_operacion cod_op = CPU;
    send_code_operacion(cod_op,socket_cliente);
    
    return socket_cliente;
}

// Función del hilo del servidor CPU-Kernel
void *funcion_hilo_servidor_cpu(void *void_args)
{
    args_hilo *args = (args_hilo *)void_args;
    t_socket_cpu *sockets = servidor_CPU_Kernel(args->log, args->config);

    if (!sockets)
    {
        log_error(args->log, "Error en la conexión con Kernel.");
        pthread_exit(NULL);
    }

    return (void *)sockets;
}

// Función del hilo cliente CPU-Memoria
void *funcion_hilo_cliente_memoria(void *void_args)
{
    args_hilo *args = (args_hilo *)void_args;
    int socket = cliente_cpu_memoria(args->log, args->config);

    if (socket == -1)
    {
        log_error(args->log, "Error en la conexión con Memoria.");
        pthread_exit(NULL);
    }

    return (void *)(intptr_t)socket;
}

// Creación de hilos CPU
t_sockets_cpu *hilos_cpu(t_log *log, t_config *config)
{


    args_hilo *args = malloc(sizeof(args_hilo));
    if (!args)
    {
        log_error(log, "Error al asignar memoria para los argumentos.");
        return NULL;
    }

    t_sockets_cpu *sockets_cpu = malloc(sizeof(t_sockets_cpu));
    if (!sockets_cpu)
    {
        free(args);
        log_error(log, "Error al asignar memoria para sockets CPU.");
        return NULL;
    }

    args->config = config;
    args->log = log;

    if (pthread_create(&hilo_servidor, NULL, funcion_hilo_servidor_cpu, (void *)args) != 0)
    {
        log_error(log, "Error al crear el hilo servidor.");
        free(args);
        free(sockets_cpu);
        return NULL;
    }

    if (pthread_create(&hilo_cliente, NULL, funcion_hilo_cliente_memoria, (void *)args) != 0)
    {
        log_error(log, "Error al crear el hilo cliente.");
        free(args);
        free(sockets_cpu);
        return NULL;
    }

    pthread_join(hilo_servidor, &socket_servidor_kernel);
    pthread_join(hilo_cliente, &socket_cliente_memoria);

    sockets_cpu->socket_servidor = (t_socket_cpu *)socket_servidor_kernel;
    sockets_cpu->socket_memoria = (intptr_t)socket_cliente_memoria;

    free(args);
    return sockets_cpu;
}

// Recepción de mensajes de Kernel Interrupt
void* recibir_kernel_interrupt(void*args){

    int noFinalizar = 0;
    while (noFinalizar != -1){
        
        log_info(log_cpu,"esperando interrupciones\n");
        //t_paquete_code_operacion* paquete = recibir_paquete_code_operacion(sockets_cpu->socket_servidor->socket_cliente_Interrupt);
        code_operacion code = recibir_code_operacion(sockets_cpu->socket_servidor->socket_cliente_Interrupt);


        log_info(log_cpu,"llega el código %d a interrupt",code);
        if(code == -1){
            log_info(log_cpu,"Conexion cerrada por Interrupt");

            return NULL;
        }
        

        switch (code)
        {
        case FIN_QUANTUM_RR:
            log_info(log_cpu,"## Llega interrupción al puerto Interrupt");
            pthread_mutex_lock(&mutex_interrupt);
            hay_interrupcion = true;
            devolucion_kernel=FIN_QUANTUM_RR;
            pthread_mutex_unlock(&mutex_interrupt);
            break;
        case DESALOJAR:
            log_info(log_cpu,"## Llega interrupción al puerto Interrupt");
            pthread_mutex_lock(&mutex_interrupt);
            hay_interrupcion = true;
            devolucion_kernel=DESALOJAR;
            pthread_mutex_unlock(&mutex_interrupt);
            send_code_operacion(OK,sockets_cpu->socket_servidor->socket_cliente_Dispatch);
            break;
        case TERMINAR_EJECUCION_MODULO:
            log_info(log_cpu, "## Llega TERMINAR_EJECUCION_MODULO");
            send_code_operacion(OK_TERMINAR,sockets_cpu->socket_servidor->socket_cliente_Interrupt);   
                     
            send_terminar_ejecucion_op_code(sockets_cpu->socket_memoria);
            op_code code = recibir_code_operacion(sockets_cpu->socket_memoria);
            if (code == OK_TERMINAR_OP_CODE){
                log_info(log_cpu, "Se va a terminar la ejecución del módulo CPU");
                sem_post(&sem_finalizacion_cpu);

                return NULL;
            }
            else{
                log_info(log_cpu,"SOY UN ESTORBO");
            }
            sem_post(&sem_finalizacion_cpu);
            return NULL;
            break;

        default:
            log_info(log_cpu,"codigo no valido recibido");
            break;
        }
        
    }

    return NULL;
}