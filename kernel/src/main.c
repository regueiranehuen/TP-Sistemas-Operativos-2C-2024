#include "includes/main.h"

    sem_t semaforo_new_ready_procesos;
    sem_t semaforo_cola_new_procesos;
    sem_t semaforo_cola_new_hilos;
    sem_t semaforo_cola_exit_procesos;
    sem_t semaforo_cola_exit_hilos;
    pthread_mutex_t mutex_pthread_join;
    pthread_mutex_t mutex_conexion_cpu;
    int estado_kernel = 1;

int main(int argc, char *argv[])
{

    t_log *log;
    t_config *config;
    sockets_kernel *sockets;
    char *archivo_pseudocodigo = argv[1];
    int tamanio_proceso = atoi(argv[2]);
    

    cola_new = queue_create();
    cola_ready = queue_create();

    log = log_create("kernel.log", "tp", true, LOG_LEVEL_TRACE);
    config = config_create("kernel.config");

    inicializar_mutex();
    sockets = hilos_kernel(log, config);
    iniciar_kernel(archivo_pseudocodigo, tamanio_proceso);
    estado_kernel = 0;
    liberar_espacio(log, config, sockets);
    return 0;
}

void liberar_espacio(t_log *log, t_config *config, sockets_kernel *sockets)
{
    config_destroy(config);
    log_destroy(log);
    close(sockets->socket_cliente_memoria);
    close(sockets->sockets_cliente_cpu->socket_Dispatch);
    close(sockets->sockets_cliente_cpu->socket_Interrupt);
    free(sockets);
    destroy_semaforo();
}

void inicializar_semaforo(){
  pthread_mutex_init(&mutex_pthread_join, NULL);
  pthread_mutex_init(&mutex_conexion_cpu, NULL);
  sem_init(&semaforo_new_ready_procesos, 0, 0);
  sem_init(&semaforo_cola_new_procesos, 0, 0);
  sem_init(&semaforo_cola_new_hilos, 0, 0);
  sem_init(&semaforo_cola_exit_procesos,0,0);
  sem_init(&semaforo_cola_exit_hilos,0,0);
}

void destroy_semaforo(){
    pthread_mutex_destroy(&mutex_pthread_join);
    pthread_mutex_destroy(&mutex_conexion_cpu);
    sem_destroy(&semaforo_new_ready_procesos);
    sem_destroy(&semaforo_cola_new_procesos);
    sem_destroy(&semaforo_cola_new_hilos);
    sem_destroy(&semaforo_cola_exit_procesos);
    sem_destroy(&semaforo_cola_exit_hilos);
}