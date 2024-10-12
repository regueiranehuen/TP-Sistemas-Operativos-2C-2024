#include "server.h"

// Lectura de configuración
void leer_config(char* path) {
    config = iniciar_config(path);

    ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    puerto_memoria = config_get_int_value(config, "PUERTO_MEMORIA");
    puerto_escucha_dispatch = config_get_int_value(config, "PUERTO_ESCUCHA_DISPATCH");
    puerto_escucha_interrupt = config_get_int_value(config, "PUERTO_ESCUCHA_INTERRUPT");
    log_level = config_get_string_value(config, "LOG_LEVEL");

    log_info(log_cpu, "Configuración del CPU cargada.");
}

// Inicialización del servidor de CPU para el Kernel
t_socket_cpu* servidor_CPU_Kernel(t_log* log, t_config* config) {
    t_socket_cpu* sockets = malloc(sizeof(t_socket_cpu));
    if (!sockets) {
        log_error(log, "Error al asignar memoria para sockets.");
        return NULL;
    }

    char puerto_str[6];
    sprintf(puerto_str, "%d", puerto_escucha_dispatch);
    socket_servidor_Dispatch = iniciar_servidor(log, puerto_str);

    sprintf(puerto_str, "%d", puerto_escucha_interrupt);
    socket_servidor_Interrupt = iniciar_servidor(log, puerto_str);

    if (socket_servidor_Dispatch == -1 || socket_servidor_Interrupt == -1) {
        log_error(log, "Error al iniciar servidores.");
        if (socket_servidor_Dispatch != -1) close(socket_servidor_Dispatch);
        if (socket_servidor_Interrupt != -1) close(socket_servidor_Interrupt);
        free(sockets);
        return NULL;
    }

    socket_cliente_Dispatch = esperar_cliente(log, socket_servidor_Dispatch);
    socket_cliente_Interrupt = esperar_cliente(log, socket_servidor_Interrupt);

    if (socket_cliente_Dispatch == -1 || socket_cliente_Interrupt == -1) {
        log_error(log, "Error al esperar cliente.");
        close(socket_servidor_Dispatch);
        close(socket_servidor_Interrupt);
        free(sockets);
        return NULL;
    }

    respuesta_Dispatch = servidor_handshake(socket_cliente_Dispatch, log);
    respuesta_Interrupt = servidor_handshake(socket_cliente_Interrupt, log);

    sockets->socket_Dispatch = socket_servidor_Dispatch;
    sockets->socket_Interrupt = socket_servidor_Interrupt;

    close(socket_cliente_Dispatch);
    close(socket_cliente_Interrupt);
    return sockets;
}

// Conexión cliente CPU-Memoria
int cliente_cpu_memoria(t_log* log, t_config* config) {
    if (!ip_memoria) {
        log_error(log, "IP de memoria no definida en la configuración.");
        return -1;
    }
    
    char puerto_str[6];
    sprintf(puerto_str, "%d", puerto_memoria);  // Línea 64
    int socket_cliente = crear_conexion(log, ip_memoria, puerto_str);
    if (socket_cliente == -1) {
        log_error(log, "Error al crear la conexión con memoria.");
        return -1;
    }

    int respuesta = cliente_handshake(socket_cliente, log);
    if (respuesta != 0) {
        log_error(log, "Error en el handshake con memoria.");
        close(socket_cliente);
        return -1;
    }

    log_info(log, "Handshake con memoria realizado correctamente.");
    return socket_cliente;
}

// Función del hilo del servidor CPU-Kernel
void* funcion_hilo_servidor_cpu(void* void_args) {
    args_hilo* args = (args_hilo*)void_args;
    t_socket_cpu* sockets = servidor_CPU_Kernel(args->log, args->config);

    if (!sockets) {
        log_error(args->log, "Error en la conexión con Kernel.");
        pthread_exit(NULL);
    }

    return (void*)sockets;
}

// Función del hilo cliente CPU-Memoria
void* funcion_hilo_cliente_memoria(void* void_args) {
    args_hilo* args = (args_hilo*)void_args;
    int socket = cliente_cpu_memoria(args->log, args->config);

    if (socket == -1) {
        log_error(args->log, "Error en la conexión con Memoria.");
        pthread_exit(NULL);
    }

    return (void*)(intptr_t)socket;
}

// Creación de hilos CPU
t_sockets_cpu* hilos_cpu(t_log* log, t_config* config) {
    args_hilo* args = malloc(sizeof(args_hilo));
    if (!args) {
        log_error(log, "Error al asignar memoria para los argumentos.");
        return NULL;
    }

    t_sockets_cpu* sockets_cpu = malloc(sizeof(t_sockets_cpu));
    if (!sockets_cpu) {
        free(args);
        log_error(log, "Error al asignar memoria para sockets CPU.");
        return NULL;
    }

    args->config = config;
    args->log = log;

    if (pthread_create(&hilo_servidor, NULL, funcion_hilo_servidor_cpu, (void*)args) != 0) {
        log_error(log, "Error al crear el hilo servidor.");
        free(args);
        free(sockets_cpu);
        return NULL;
    }

    if (pthread_create(&hilo_cliente, NULL, funcion_hilo_cliente_memoria, (void*)args) != 0) {
        log_error(log, "Error al crear el hilo cliente.");
        free(args);
        free(sockets_cpu);
        return NULL;
    }

    pthread_join(hilo_servidor, &socket_servidor_kernel);
    pthread_join(hilo_cliente, &socket_cliente_memoria);

    sockets_cpu->socket_servidor = (t_socket_cpu*)socket_servidor_kernel;
    sockets_cpu->socket_cliente = (intptr_t)socket_cliente_memoria;

    free(args);
    return sockets_cpu;
}

// Recepción de mensajes de Kernel Interrupt
void recibir_kernel_interrupt(int socket_cliente_Interrupt) {
    enviar_string(socket_cliente_Interrupt, "Mensaje desde CPU Interrupt", MENSAJE);

    int noFinalizar = 0;
    while (noFinalizar != -1) {
        int codOperacion = recibir_operacion(socket_cliente_Interrupt);
        switch (codOperacion) {
            case INTERRUPCION_USUARIO:
                tid_interrupt = recibir_entero_uint32(socket_cliente_Interrupt, log_cpu);
                hay_interrupcion = 1;
                es_por_usuario = 1;
                break;
            case -1:
                noFinalizar = codOperacion;
                break;
            default:
                break;
        }
    }
}
