#include "main.h"

int main(int argc, char** argv) {

    log_cpu = log_create("CPU.log", "tp", true, LOG_LEVEL_TRACE);
    
    inicializar_estructuras();
    inicializar_mutex();
    inicializar_semaforos();


    sockets_cpu = hilos_cpu(log_cpu, config);

    iniciar_cpu(sockets_cpu->socket_servidor->socket_Dispatch,sockets_cpu->socket_servidor->socket_Interrupt);

    sem_wait(&sem_finalizacion_cpu);
    terminar_programa();

    return 0;

    //aca deberia empezar a ejecutar cada parte del ciclo de instrucciones, funcExecute, y mmu

}

void iniciar_cpu(int socket_dispatch,int socket_interrupt){

    pthread_t hilo_ciclo_instruccion;
    pthread_t hilo_atiende_interrupt;

    int resultado;
    resultado=pthread_create(&hilo_ciclo_instruccion,NULL,ciclo_de_instruccion,&socket_dispatch);

    if (resultado != 0)
    {
        log_error(log_cpu, "Error al crear el hilo que atiende dispatch en cpu");
    }

    resultado=pthread_create(&hilo_atiende_interrupt,NULL,recibir_kernel_interrupt,&socket_interrupt);

    if (resultado != 0)
    {
        log_error(log_cpu, "Error al crear el hilo que atiende interrupt en cpu");

    }

    resultado=pthread_create(&hilo_ciclo_instruccion,NULL,ciclo_de_instruccion,NULL);

    pthread_detach(hilo_ciclo_instruccion);
    pthread_detach(hilo_atiende_interrupt);
    
    
}