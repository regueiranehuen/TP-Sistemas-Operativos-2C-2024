#include "mmu.h"
#include "server.h"

uint32_t traducir_direccion_logica(t_contexto_tid*contexto_tid,t_contexto_pid_send *contexto_pid,uint32_t direccion_logica) { 
    uint32_t base = contexto_pid->base;
    uint32_t limite = contexto_pid->limite;  


    log_info(log_cpu, "Base de la partición: %d", base);
    log_info(log_cpu, "Límite de la partición: %d", limite);

    if (base + direccion_logica >= limite || direccion_logica < 0) {//suponiendo que el limite es el primer byte no valido de la particion
        log_error(log_cpu, "Segmentation Fault: Dirección lógica fuera del límite (%d)", direccion_logica);
        enviar_registros_a_actualizar(sockets_cpu->socket_memoria,contexto_tid->registros,contexto_pid->pid,contexto_tid->tid);
        send_segmentation_fault(sockets_cpu->socket_servidor->socket_cliente_Dispatch);
        sem_wait(&sem_ok_o_interrupcion);
        return -1;  // Retorna error
    }

    uint32_t direccion_fisica = base + direccion_logica;
    log_info(log_cpu, "Dirección física calculada: %d", direccion_fisica);

    return direccion_fisica;
}

/*
int traducir_direccion_logica(t_contexto_tid*contexto_tid,t_contexto_pid_send *contexto_pid,int direccion_logica) { 
    uint32_t base = contexto_pid->base;
    uint32_t limite = contexto_pid->limite;  


    log_info(log_cpu, "Base de la partición: %d", base);
    log_info(log_cpu, "Límite de la partición: %d", limite);

    //VER SUS MODIFICACIONES AL MMU
    if (direccion_logica < 0 || direccion_logica >= limite) { // ver si borrar 1er cond porque uint son enteros sin signo
        log_error(log_cpu, "Segmentation Fault: Dirección lógica fuera del límite (%d)", direccion_logica);
        enviar_registros_a_actualizar(sockets_cpu->socket_memoria,contexto_tid->registros,contexto_pid->pid,contexto_tid->tid);
        send_segmentation_fault(sockets_cpu->socket_servidor->socket_cliente_Dispatch);
        return -1;  // Retorna error
    }

    int direccion_fisica = base + direccion_logica;
    log_info(log_cpu, "Dirección física calculada: %d", direccion_fisica);

    return direccion_fisica;
}

*/
