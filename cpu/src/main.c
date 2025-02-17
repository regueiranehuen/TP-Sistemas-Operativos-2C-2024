#include "main.h"

int main(int argc, char** argv) {

    if (argc <= 1 || argc > 2)
    {
        
        printf("Ingrese ./bin/cpu <path del config>\n");

        return -1;
    }
    
    inicializar_mutex();
    inicializar_semaforos();

    char* path_config = argv[1];

    leer_config(path_config);

    t_log_level log_level_int = log_level(config);

    
    log_cpu = log_create("CPU.log", "tp", true, log_level_int);

    pthread_mutex_lock(&mutex_logs);
    log_info(log_cpu, "Configuración del CPU cargada.");
    pthread_mutex_unlock(&mutex_logs);
    
    sockets_cpu = hilos_cpu(log_cpu, config);

    iniciar_cpu();

    sem_wait(&sem_finalizacion_cpu);
    sem_wait(&sem_finalizacion_cpu);
    
    terminar_programa();

    return 0;
}
