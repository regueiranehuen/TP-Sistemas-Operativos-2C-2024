#include "main.h"

int main(int argc, char** argv) {

    
    inicializar_estructuras();
    inicializar_mutex();
    inicializar_semaforos();

    t_log_level log_level_int = log_level(config);
    log_cpu = log_create("CPU.log", "tp", true, log_level_int);


    //FALTA CONFIG

    sockets_cpu = hilos_cpu(log_cpu, config);

    iniciar_cpu();

    sem_wait(&sem_finalizacion_cpu);
    
    terminar_programa();

    return 0;
}
