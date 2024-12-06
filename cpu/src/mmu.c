#include "mmu.h"
#include "server.h"

uint32_t* traducir_direccion_logica(t_contexto_tid*contexto_tid,t_contexto_pid_send *contexto_pid,uint32_t direccion_logica) { 
    uint32_t base = contexto_pid->base;
    uint32_t limite = contexto_pid->limite;  

    pthread_mutex_lock(&mutex_logs);
    log_info(log_cpu, "Base de la partición: %d", base);
    log_info(log_cpu, "Límite de la partición: %d", limite);
    pthread_mutex_unlock(&mutex_logs);

    if (base + direccion_logica + sizeof(uint32_t) >= limite || direccion_logica < 0) { // Base + desplazamiento + size del registro < límite para no entrar en seg fault
        pthread_mutex_lock(&mutex_logs);
        log_error(log_cpu, "Segmentation Fault: Dirección lógica fuera del límite (%d)", direccion_logica);
        pthread_mutex_unlock(&mutex_logs);

        enviar_registros_a_actualizar(sockets_cpu->socket_memoria,contexto_tid->registros,contexto_tid->pid,contexto_tid->tid);
        recibir_code_operacion(sockets_cpu->socket_memoria);
        
        send_segmentation_fault(sockets_cpu->socket_servidor->socket_cliente_Dispatch);

        pthread_mutex_lock(&mutex_logs);
        log_info(log_cpu,"Envio SEGMENTATION_FAULT");
        pthread_mutex_unlock(&mutex_logs);

        recibir_code_operacion(sockets_cpu->socket_servidor->socket_cliente_Dispatch);
        
        return NULL;  // Retorna error
    }
    uint32_t *direccion_fisica = malloc(sizeof(uint32_t));
    *direccion_fisica = base + direccion_logica;

    pthread_mutex_lock(&mutex_logs);
    log_info(log_cpu, "Dirección física calculada: %d", *direccion_fisica);
    pthread_mutex_unlock(&mutex_logs);
    
    return direccion_fisica;
}


