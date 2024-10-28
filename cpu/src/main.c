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

    //aca deberia empezar a ejecutar cada parte del ciclo de instrucciones, funcExecute, y mmu

}

void iniciar_cpu(){
    pthread_t hilo_atiende_dispatch;
    pthread_t hilo_atiende_interrupt;
    pthread_t hilo_ciclo_instruccion;

    int resultado;
    resultado=pthread_create(&hilo_atiende_dispatch,NULL,recibir_kernel_dispatch,NULL);

    if (resultado != 0)
    {
        log_error(log_cpu, "Error al crear el hilo que atiende dispatch en cpu");
    }

    resultado=pthread_create(&hilo_atiende_interrupt,NULL,recibir_kernel_interrupt,NULL);

    if (resultado != 0)
    {
        log_error(log_cpu, "Error al crear el hilo que atiende interrupt en cpu");

    }

    resultado=pthread_create(&hilo_ciclo_instruccion,NULL,ciclo_de_instruccion,NULL);

    if (resultado != 0)
    {
        log_error(log_cpu, "Error al crear el hilo que atiende interrupt en cpu");

    }

    pthread_detach(hilo_atiende_dispatch);
    pthread_detach(hilo_atiende_interrupt);
    pthread_detach(hilo_ciclo_instruccion);
    
}