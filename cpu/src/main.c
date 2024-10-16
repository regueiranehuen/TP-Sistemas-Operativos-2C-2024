#include "main.h"

int main(int argc, char** argv) {
    inicializar_estructuras();
    leer_config(argv[1]);
    log_info(log_cpu, "INICIA EL MODULO DE CPU");

    sockets_cpu = hilos_cpu(log_cpu, config);
    ciclo_de_instruccion(log_cpu);
    terminar_programa();

    return 0;
}

void liberarMemoria(t_sockets_cpu* sockets, t_log* log, t_config* config) {
    if (sockets == NULL || sockets->socket_memoria == -1 ||
        sockets->socket_servidor == NULL || 
        sockets->socket_servidor->socket_Dispatch == -1 || 
        sockets->socket_servidor->socket_Interrupt == -1) {
        
        log_info(log, "Error en los sockets de CPU");
    } else {
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
    liberarMemoria(sockets_cpu, log_cpu, config); // Llama a liberarMemoria
    liberar_contexto(contexto); // Liberar el contexto

    if (pcb_salida != NULL) {
        free(pcb_salida);
    }
    
    log_info(log_cpu, "Estructuras liberadas");
}           

void inicializar_estructuras() {
    log_cpu = log_create("CPU.log", "tp", true, LOG_LEVEL_TRACE);
    config = config_create("CPU.config");
    
    contexto = inicializar_contexto(0);
    sockets_cpu = malloc(sizeof(t_sockets_cpu));
    pcb_salida = malloc(sizeof(t_pcb_exit));

    log_info(log_cpu, "Estructuras inicializadas");
}