#include "main.h"

int main(int argc, char** argv) {

    log_cpu = log_create("CPU.log", "tp", true, LOG_LEVEL_TRACE);
    config = config_create("CPU.config");
    leer_config(argv[1]);
    
    inicializar_estructuras();
    inicializar_mutex();
    inicializar_semaforos();


    sockets_cpu = hilos_cpu(log_cpu, config);

    iniciar_cpu(sockets_cpu->socket_servidor->socket_Dispatch,sockets_cpu->socket_servidor->socket_Interrupt);

    // Acá hay que esperar a que termine la cpu de ejecutar
    terminar_programa();

    return 0;

    //aca deberia empezar a ejecutar cada parte del ciclo de instrucciones, funcExecute, y mmu

}

void iniciar_cpu(int socket_dispatch,int socket_interrupt){
    pthread_t hilo_atiende_dispatch;
    pthread_t hilo_atiende_interrupt;

    int resultado;
    resultado=pthread_create(&hilo_atiende_dispatch,NULL,recibir_kernel_dispatch,&socket_dispatch);

    if (resultado != 0)
    {
        log_error(log_cpu, "Error al crear el hilo que atiende dispatch en cpu");
    }

    resultado=pthread_create(&hilo_atiende_interrupt,NULL,recibir_kernel_interrupt,&socket_interrupt);

    if (resultado != 0)
    {
        log_error(log_cpu, "Error al crear el hilo que atiende interrupt en cpu");

    }

    pthread_detach(hilo_atiende_dispatch);
    pthread_detach(hilo_atiende_interrupt);

    
}