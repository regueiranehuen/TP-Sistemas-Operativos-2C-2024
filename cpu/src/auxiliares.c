#include "auxiliares.h"

void inicializar_semaforos(){
    sem_init(&sem_syscall_interrumpida_o_finalizada,0,0);
}

void inicializar_estructuras() {
    log_cpu = log_create("CPU.log", "tp", true, LOG_LEVEL_TRACE);
    config = config_create("CPU.config");
    

    sockets_cpu = malloc(sizeof(t_sockets_cpu));


    log_info(log_cpu, "Estructuras inicializadas");
}


void inicializar_mutex(){
    inicializar_mutex_compartido_entre_procesos(&mutex_conexion_kernel_dispatch);
    inicializar_mutex_compartido_entre_procesos(&mutex_conexion_kernel_interrupt);
    pthread_mutex_init(&mutex_tid_pid_exec,NULL);
    pthread_mutex_init(&mutex_interrupt,NULL);
}


void inicializar_mutex_compartido_entre_procesos(pthread_mutex_t* mutex){
    // Inicializar el mutex con atributos compartidos entre procesos
    pthread_mutexattr_t attr_conexion_kernel_cpu;
    pthread_mutexattr_setpshared(&attr_conexion_kernel_cpu, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(mutex, &attr_conexion_kernel_cpu);
    pthread_mutexattr_destroy(&attr_conexion_kernel_cpu); // Destruir los atributos después de inicializar el mutex
}

void destruir_mutex(){
    pthread_mutex_destroy(&mutex_conexion_kernel_dispatch);
    pthread_mutex_destroy(&mutex_conexion_kernel_interrupt);
    pthread_mutex_destroy(&mutex_tid_pid_exec);
    pthread_mutex_destroy(&mutex_interrupt);
}

void destruir_semaforos(){
    sem_destroy(&sem_syscall_interrumpida_o_finalizada);
}

void liberarMemoria(t_sockets_cpu * sockets,t_log* log, t_config* config){

    if (sockets == NULL || sockets->socket_memoria == -1 ||
        sockets->socket_servidor == NULL || 
        sockets->socket_servidor->socket_Dispatch == -1 || 
        sockets->socket_servidor->socket_Interrupt == -1) {

        log_info(log, "Error en los sockets de cpu");
        }
        else{
    close(sockets->socket_memoria);
    close(sockets->socket_servidor->socket_Dispatch);
    close(sockets->socket_servidor->socket_Interrupt);
    }
    if (sockets != NULL) {
            if (sockets->socket_servidor != NULL) {
                free(sockets->socket_servidor);
            }
            free(sockets);
        }
    config_destroy(config);
    log_destroy(log);
}

void terminar_programa() {
    liberarMemoria(sockets_cpu, log_cpu, config); 
    destruir_mutex();
    destruir_semaforos();
    log_info(log_cpu, "Estructuras liberadas");
}  