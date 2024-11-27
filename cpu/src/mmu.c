#include "mmu.h"
#include "server.h"

uint32_t* traducir_direccion_logica(t_contexto_tid*contexto_tid,t_contexto_pid_send *contexto_pid,uint32_t direccion_logica) { 
    uint32_t base = contexto_pid->base;
    uint32_t limite = contexto_pid->limite;  

    log_info(log_cpu,"rraaah pid %d",contexto_pid->pid);

    log_info(log_cpu, "Base de la partición: %d", base);
    log_info(log_cpu, "Límite de la partición: %d", limite);

    if (base + direccion_logica + sizeof(uint32_t) >= limite || direccion_logica < 0) { // Base + desplazamiento + size del registro < límite para no entrar en seg fault
        log_error(log_cpu, "Segmentation Fault: Dirección lógica fuera del límite (%d)", direccion_logica);
        enviar_registros_a_actualizar(sockets_cpu->socket_memoria,contexto_tid->registros,contexto_tid->pid,contexto_tid->tid);
        code_operacion code = recibir_code_operacion(sockets_cpu->socket_memoria);
        if (code ==OK){
            log_info(log_cpu,"PROCEDA");
        }
        else{
            log_info(log_cpu,"CUACK CUACK");
        }
        send_segmentation_fault(sockets_cpu->socket_servidor->socket_cliente_Dispatch);
        sem_wait(&sem_ok_o_interrupcion); // Problema: Acá en colas multinivel puede pasar que llegue fin de quantum pero nosotros lo tomemos como ok al seg fault 
        return NULL;  // Retorna error
    }
    uint32_t *direccion_fisica = malloc(sizeof(uint32_t));
    *direccion_fisica = base + direccion_logica;
    log_info(log_cpu, "Dirección física calculada: %d", *direccion_fisica);
    
    return direccion_fisica;
}


