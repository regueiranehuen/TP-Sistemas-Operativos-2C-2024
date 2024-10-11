#include "mmu.h"
#include "server.h"

int traducir_direccion_logica(int direccion_logica) {
    uint32_t base = contexto->base;
    uint32_t limite = contexto->limite;  

    log_info(log_cpu, "Base de la partición: %d", base);
    log_info(log_cpu, "Límite de la partición: %d", limite);

    if (direccion_logica >= limite) {
        log_error(log_cpu, "Segmentation Fault: Dirección lógica fuera del límite (%d)", direccion_logica);
        contexto->razon_salida = SEGMENTATION_FAULT;
        actualizar_contexto_en_memoria(contexto);
        notificar_kernel_terminacion(contexto->tid, SEGMENTATION_FAULT);
        return -1;  // Retorna error
    }

    int direccion_fisica = base + direccion_logica;
    log_info(log_cpu, "Dirección física calculada: %d", direccion_fisica);

    return direccion_fisica;
}

void actualizar_contexto_en_memoria(t_contexto *contexto) {
    t_paquete *paquete = crear_paquete_op(ACTUALIZAR_CONTEXTO_MEMORIA);
    
    agregar_entero_a_paquete(paquete, contexto->tid);
    agregar_entero_a_paquete(paquete, contexto->razon_salida);
    
    log_info(log_cpu, "Actualizando contexto en memoria para TID: %d con razón de salida: %d", contexto->tid, contexto->razon_salida);
    

    enviar_paquete(paquete, conexion_memoria);
    
    eliminar_paquete(paquete);
    
    log_info(log_cpu, "Contexto actualizado correctamente en memoria para TID: %d", contexto->tid);
}

void notificar_kernel_terminacion(int tid, int razon_salida) {
    t_paquete *paquete = crear_paquete_op(TERMINACION_PROCESO);
    

    agregar_entero_a_paquete(paquete, tid);
    agregar_entero_a_paquete(paquete, razon_salida);
    
    log_info(log_cpu, "Notificando al Kernel la terminación del TID: %d con razón de salida: %d", tid, razon_salida);
    
    enviar_paquete(paquete, conexion_kernel);
    
    eliminar_paquete(paquete);
    
    log_info(log_cpu, "Notificación de terminación enviada al Kernel para TID: %d", tid);
}