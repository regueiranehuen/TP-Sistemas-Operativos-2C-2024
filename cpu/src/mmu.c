#include "mmu.h"
#include "server.h"

uint32_t traducir_direccion_logica(t_contexto_pid *contexto,uint32_t direccion_logica) { 
    uint32_t base = contexto->base;
    uint32_t limite = contexto->limite;  


    log_info(log_cpu, "Base de la partición: %d", base);
    log_info(log_cpu, "Límite de la partición: %d", limite);

    if (direccion_logica >= limite || direccion_logica < 0) {
        log_error(log_cpu, "Segmentation Fault: Dirección lógica fuera del límite (%d)", direccion_logica);
        pcb_salida->motivo = SEGMENTATION_FAULT;
        enviar_contexto_a_memoria(contexto);
        notificar_kernel_terminacion(contexto->tid, SEGMENTATION_FAULT);
        return -1;  // Retorna error
    }

    int direccion_fisica = base + direccion_logica;
    log_info(log_cpu, "Dirección física calculada: %d", direccion_fisica);

    return direccion_fisica;
}


void notificar_kernel_terminacion(int tid, code_operacion code) { // TODO REVISAR EN KERNEL
    
    log_info(log_cpu, "Notificando al Kernel la terminación del TID: %d con razón de salida: %d", tid, razon_salida);
    
    send_operacion_tid(code, sockets_cpu->socket_servidor->socket_Interrupt);

    log_info(log_cpu, "Notificación de terminación enviada al Kernel para TID: %d", tid);
}