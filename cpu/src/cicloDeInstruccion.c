#include "cicloDeInstruccion.h"

void ciclo_de_instruccion() {

    seguir_ejecutando=1;

    while(seguir_ejecutando) {

        t_instruccion* instruccion = fetch(contexto->tid,contexto->pc);
        op_code nombre_instruccion = decode(instruccion);
        execute(nombre_instruccion, instruccion);
        //dentro del pcb esta el pc con demas registris
        //pcb->pc++;
        if(seguir_ejecutando) { 
        checkInterrupt(contexto->tid);
        }
}
}

t_instruccion* fetch(uint32_t tid, uint32_t pc) {

    pedir_instruccion_memoria(tid, pc, log_cpu);
    log_info(log_cpu, "PID: %i - FETCH - Program Counter: %i", tid, pc);
    t_instruccion* instruccion = malloc(sizeof(t_instruccion));
    op_code codigo = recibir_operacion(sockets->socket_cliente); // TODO ver como modelar la operacion
    if(codigo == READY){
        log_info(log_cpu, "COPOP: %i", codigo);
        instruccion = recibir_instruccion(sockets->socket_cliente); // 
    }else{
        return -1;
    }
  return instruccion;
    
}

void pedir_instruccion_memoria(uint32_t tid, uint32_t pc, t_log *logg){
    t_paquete* paquete = crear_paquete_op(PEDIR_INSTRUCCION_MEMORIA);
    agregar_entero_a_paquete(paquete,tid);
    agregar_entero_a_paquete(paquete,pc);
    
    //log_info(logg, "serializacion %i %i", tid, pc); ya esta el log
    enviar_paquete(paquete,sockets->socket_cliente);
    eliminar_paquete(paquete);

}

op_code decode() {
    if (strcmp(instruccion->parametros1, "SET") == 0) {
        return SET;
    } else if (strcmp(instruccion->parametros1, "READ_MEM") == 0) {
        return READ_MEM;
    } else if (strcmp(instruccion->parametros1, "WRITE_MEM") == 0) {
        return WRITE_MEM;
    } else if (strcmp(instruccion->parametros1, "SUM") == 0) {
        return SUM;
    } else if (strcmp(instruccion->parametros1, "SUB") == 0) {
        return SUB;
    } else if (strcmp(instruccion->parametros1, "JNZ") == 0) {
        return JNZ;
    } else if (strcmp(instruccion->parametros1, "LOG") == 0) {
        return LOG;
    } else if (strcmp(instruccion->parametros1, "DUMP_MEMORY") == 0) {
        return DUMP_MEMORY;
    } else if (strcmp(instruccion->parametros1, "IO") == 0) {
        return IO;
    } else if (strcmp(instruccion->parametros1, "PROCESS_CREATE") == 0) {
        return PROCESS_CREATE ;
    } else if (strcmp(instruccion->parametros1, "THREAD_CREATE") == 0) {
        return THREAD_CREATE ;
    } else if (strcmp(instruccion->parametros1, "THREAD_JOIN") == 0) {
        return THREAD_JOIN ;
    } else if (strcmp(instruccion->parametros1, "THREAD_CANCEL") == 0) {
        return THREAD_CANCEL ;
    } else if (strcmp(instruccion->parametros1, "MUTEX_CREATE") == 0) {
        return MUTEX_CREATE ;
    } else if (strcmp(instruccion->parametros1, "MUTEX_LOCK") == 0) {
        return MUTEX_LOCK ;
    } else if (strcmp(instruccion->parametros1, "MUTEX_UNLOCK") == 0) {
        return MUTEX_UNLOCK ;
    } else if (strcmp(instruccion->parametros1, "THREAD_EXIT") == 0) {
        return THREAD_EXIT;
    } else if (strcmp(instruccion->parametros1, "IO_FS_READ") == 0) {
        return PROCESS_EXIT;
    } 
    
    return -1; // Código de operación no válido

}


void execute(op_code instruccion_nombre, t_instruccion* instruccion) {
    switch (instruccion_nombre) {
        case SET:
            log_info(log_cpu, "INSTRUCCION :%s - PARAMETRO 1: %s - PARAMETRO 2: %s", instruccion->parametros1, instruccion->parametros2, instruccion->parametros3);
            funcSet(instruccion);
            break;
        case SUM:
            log_info(log_cpu, "INSTRUCCION :%s - PARAMETRO 1: %s - PARAMETRO 2: %s", instruccion->parametros1, instruccion->parametros2, instruccion->parametros3);
            funcSum(instruccion);
            break;
        case SUB:
            log_info(log_cpu, "INSTRUCCION :%s - PARAMETRO 1: %s - PARAMETRO 2: %s", instruccion->parametros1, instruccion->parametros2, instruccion->parametros3);
            funcSub(instruccion);
            break;
        case JNZ:
            log_info(log_cpu, "INSTRUCCION :%s - PARAMETRO 1: %s - PARAMETRO 2: %s", instruccion->parametros1, instruccion->parametros2, instruccion->parametros3);
            funcJnz(instruccion);
            break;
        case READ_MEM:
            log_info(log_cpu, "INSTRUCCION :%s - PARAMETRO 1: %s - PARAMETRO 2: %s", instruccion->parametros1, instruccion->parametros2, instruccion->parametros3);
            funcREAD_MEM(instruccion);
            esperar_devolucion_pcb();
            break;
        case WRITE_MEM:
            log_info(log_cpu, "INSTRUCCION :%s - PARAMETRO 1: %s", instruccion->parametros1, instruccion->parametros2);
            funcWait(instruccion);
            funWRITE_MEM();
            break;
        default:
            log_info(log_cpu, "Instrucción desconocida\n");
            break;
    }

    
}

void checkInterrupt(uint32_t tid){
    if (hay_interrupcion){
        hay_interrupcion = 0;
        if(contexto->tid == tid_interrupt){
            seguir_ejecutando = 0;
            if(!es_por_usuario){
                enviar_contexto(sockets->socket_Dispatch, contexto,INTERRUPCION);
            }else{
                enviar_contexto(sockets->socket_Dispatch, contexto,INTERRUPCION_USUARIO);
            }
        }
    }
}