#include "main.h"

int main(int argc, char** argv) {

    log_cpu = log_create("CPU.log", "tp", true, LOG_LEVEL_TRACE);
    config = config_create("CPU.config");
    leer_config(argv[1]);

    sockets_cpu = hilos_cpu(log, config);

    liberarMemoria(sockets_cpu,log,config);

    return 0;

    //aca deberia empezar a ejecutar cada parte del ciclo de instrucciones, funcExecute, y mmu




}

void liberarMemoria(t_sockets_cpu * sockets,t_log* log, t_config* config){

    if (sockets == NULL || sockets->socket_cliente == -1 ||
        sockets->socket_servidor == NULL || 
        sockets->socket_servidor->socket_Dispatch == -1 || 
        sockets->socket_servidor->socket_Interrupt == -1) {

        log_info(log, "Error en los sockets de cpu");
        }
        else{
    close(sockets->socket_cliente);
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