#include "main.h"

int main(int argc, char** argv) {

    log_cpu = log_create("CPU.log", "tp", true, LOG_LEVEL_TRACE);
    
    inicializar_estructuras();
    inicializar_mutex();
    inicializar_semaforos();


    sockets_cpu = hilos_cpu(log_cpu, config);

    iniciar_cpu();

    sem_wait(&sem_finalizacion_cpu);
    
    terminar_programa();

    return 0;
}
