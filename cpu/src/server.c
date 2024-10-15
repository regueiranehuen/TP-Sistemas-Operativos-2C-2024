#include "server.h"

t_log* log_cpu = NULL;
t_config* config = NULL;
t_sockets_cpu* sockets_cpu = NULL; // ¿
t_contexto* contexto = NULL;
t_pcb_exit* pcb_salida = NULL;

char* ip_memoria = NULL;
int puerto_memoria = 0;
int puerto_escucha_dispatch = 0;
int puerto_escucha_interrupt = 0;
char* log_level = NULL;

int socket_servidor_Dispatch = 0, socket_servidor_Interrupt = 0;
int socket_cliente_Dispatch = 0, socket_cliente_Interrupt = 0;
int respuesta_Dispatch = 0, respuesta_Interrupt = 0;

t_socket_cpu* sockets = NULL; // ?

pthread_t hilo_servidor;
pthread_t hilo_cliente;
void* socket_servidor_kernel = NULL;
void* socket_cliente_memoria = NULL;

uint32_t tid_interrupt = 0;
int hay_interrupcion = 0;
int es_por_usuario = 0;

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

t_config *iniciar_config(char *path) {
    t_config *nuevo_config = config_create(path);
    if (nuevo_config == NULL) {
        printf("No se puede crear la config");
        exit(1);
    }
    return nuevo_config;
}

void liberar_config(t_config* config) {
    if (config != NULL) {
        config_destroy(config);
    }
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
    sockets_cpu->socket_memoria = (intptr_t)socket_cliente_memoria;

    free(args);
    return sockets_cpu;
}

// Recepción de mensajes de Kernel Interrupt
void recibir_kernel_interrupt(int socket_cliente_Interrupt) {
    enviar_string(socket_cliente_Interrupt, "Mensaje desde CPU Interrupt", MENSAJE);

    int noFinalizar = 0;
    while (noFinalizar != -1) {

        t_paquete_code_operacion* paquete=recibir_paquete_code_operacion(socket_cliente_Interrupt); 

        //int codOperacion = recibir_operacion(socket_cliente_Interrupt); ///////////
        /*switch (codOperacion) {
            case INTERRUPCION_USUARIO:
                //recibir_
                tid_interrupt = recibir_entero_uint32(socket_cliente_Interrupt, log_cpu);
                hay_interrupcion = 1;
                es_por_usuario = 1;
                break;
            case -1:
                noFinalizar = codOperacion;
                break;
            default:
                break;
        }*/

        if (es_interrupcion_usuario(paquete->code)){ // No entiendo a qué se refieren con interrupción de usuario
            tid_interrupt = recepcionar_int_code_op(paquete); // Hay que ver si hay algun problema igualando uint_32 con int
            hay_interrupcion = 1;
            es_por_usuario = 1;
        }
        else if (paquete->code == TERMINAR){
            noFinalizar=-1;
        }

    }
}

bool es_interrupcion_usuario(code_operacion code){
    return (code == DUMP_MEMORIA || code == FIN_QUANTUM_RR || code == THREAD_INTERRUPT);
}


t_contexto* crear_contexto(int tid, uint32_t base, uint32_t limite) {
    t_contexto* nuevo_contexto = malloc(sizeof(t_contexto));
    if (nuevo_contexto == NULL) {
        return NULL;
    }
    nuevo_contexto->tid = tid;
    nuevo_contexto->pc = 0;
    nuevo_contexto->registros = malloc(sizeof(t_registros_cpu));
    if (nuevo_contexto->registros == NULL) {
        free(nuevo_contexto);
        return NULL;
    }
    nuevo_contexto->registros->base = base;
    nuevo_contexto->registros->limite = limite;
    memset(nuevo_contexto->registros, 0, sizeof(t_registros_cpu));
    return nuevo_contexto;
}

int existe_contexto(t_contexto_pid* contexto_pid, int tid) {
    for (int i = 0; i < list_size(contexto_pid->contextos_tids); i++) {
        t_contexto_tid* contexto_tid = list_get(contexto_pid->contextos_tids, i);
        if (contexto_tid->tid == tid) {
            return 1;
        }
    }
    return 0;
}

int agregar_contexto_tid(t_contexto_pid* contexto_pid, int tid) {
    if (existe_contexto(contexto_pid, tid)) {
        return -1;
    }
    
    t_contexto_tid* nuevo_contexto_tid = malloc(sizeof(t_contexto_tid));
    if (nuevo_contexto_tid == NULL) {

        return -1;
    }
    nuevo_contexto_tid->tid = tid;
    nuevo_contexto_tid->contexto_ejecucion = crear_contexto(tid, contexto_pid->base, contexto_pid->limite);
    
    if (nuevo_contexto_tid->contexto_ejecucion == NULL) {
        free(nuevo_contexto_tid);
        return -1;
    }
    
    list_add(contexto_pid->contextos_tids, nuevo_contexto_tid);
    return 0;
}

// Funciones para inicializar estructuras
t_registros_cpu* inicializar_registros_cpu() {
    t_registros_cpu* registros = malloc(sizeof(t_registros_cpu));
    if (registros != NULL) {
        registros->AX = 0;
        registros->BX = 0;
        registros->CX = 0;
        registros->DX = 0;
        registros->EX = 0;
        registros->FX = 0;
        registros->GX = 0;
        registros->HX = 0;
        registros->base = 0;
        registros->limite = 0;
    }
    return registros;
}

t_contexto* inicializar_contexto(int tid) {
    t_contexto* contexto = malloc(sizeof(t_contexto));
    if (contexto != NULL) {
        contexto->tid = tid;
        contexto->pc = 0;
        contexto->registros = inicializar_registros_cpu();
    }
    return contexto;
}

t_contexto_pid* inicializar_contexto_pid(int pid) {
    t_contexto_pid* contexto_pid = malloc(sizeof(t_contexto_pid));
    if (contexto_pid != NULL) {
        contexto_pid->pid = pid;
        contexto_pid->contextos_tids = list_create();
        contexto_pid->base = 0;
        contexto_pid->limite = 0;
    }
    return contexto_pid;
}

t_contexto_tid* inicializar_contexto_tid(int tid) {
    t_contexto_tid* contexto_tid = malloc(sizeof(t_contexto_tid));
    if (contexto_tid != NULL) {
        contexto_tid->tid = tid;
        contexto_tid->contexto_ejecucion = inicializar_contexto(tid);
    }
    return contexto_tid;
}

t_pcb* inicializar_pcb(t_contexto* contexto) {
    t_pcb* pcb = malloc(sizeof(t_pcb));
    if (pcb != NULL) {
        pcb->contexto = contexto;
        pcb->quantum_utilizado = 0;
    }
    return pcb;
}

// Funciones para liberar estructuras
void liberar_registros_cpu(t_registros_cpu* registros) {
    if (registros != NULL) {
        free(registros);
    }
}

void liberar_contexto(t_contexto* contexto) {
    if (contexto != NULL) {
        liberar_registros_cpu(contexto->registros);
        free(contexto);
    }
}

void liberar_contexto_pid(t_contexto_pid* contexto_pid) {
    if (contexto_pid != NULL) {

        list_destroy_and_destroy_elements(contexto_pid->contextos_tids, (void*)liberar_contexto);
        free(contexto_pid);
    }
}

void liberar_contexto_tid(t_contexto_tid* contexto_tid) {
    if (contexto_tid != NULL) {
        liberar_contexto(contexto_tid->contexto_ejecucion);
        free(contexto_tid);
    }
}

void liberar_pcb(t_pcb* pcb) {
    if (pcb != NULL) {
        liberar_contexto(pcb->contexto);

        free(pcb);
    }
}