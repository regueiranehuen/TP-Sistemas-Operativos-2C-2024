#include "mmu.h"
#include "server.h"

uint32_t traducir_direccion_logica(uint32_t direccion_logica) {
    uint32_t base = contexto->registros->base;
    uint32_t limite = contexto->registros->limite;

    log_info(log_cpu, "Base de la partición: %d", base);
    log_info(log_cpu, "Límite de la partición: %d", limite);

    if (direccion_logica >= limite) {
        log_error(log_cpu, "Segmentation Fault: Dirección lógica fuera del límite (%d)", direccion_logica);
        
        pcb_salida->motivo = OUT_OF_MEMORY; //ver como hacer esto ya que motivo no puede ser segmentation fault
        
        notificar_kernel_terminacion(contexto->tid, SEGMENTATION_FAULT);
        return -1;  
    }

    uint32_t direccion_fisica = base + direccion_logica;
    log_info(log_cpu, "Dirección física calculada: %d", direccion_fisica);

    return direccion_fisica;
}

void notificar_kernel_terminacion(int tid, int razon_salida) {
    t_paquete *paquete = crear_paquete_op(TERMINACION_PROCESO);
    
    agregar_entero_a_paquete(paquete, tid);
    agregar_entero_a_paquete(paquete, razon_salida);
    
    log_info(log_cpu, "Notificando al Kernel la terminación del TID: %d con razón de salida: %d", tid, razon_salida);
    
    enviar_paquete(paquete, sockets_cpu->socket_servidor->socket_Interrupt);
    
    eliminar_paquete(paquete);
    
    log_info(log_cpu, "Notificación de terminación enviada al Kernel para TID: %d", tid);
}