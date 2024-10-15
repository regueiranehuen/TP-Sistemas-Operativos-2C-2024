#include "cicloDeInstruccion.h"
#include "funcExecute.h"
#include "mmu.h"
#include "server.h"

t_instruccion instruccion;
bool seguir_ejecutando;

void ciclo_de_instruccion(t_log* loggs) {

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

t_instruccion* fetch(uint32_t tid, uint32_t pc){

    pedir_instruccion_memoria(tid, pc, log_cpu);
    log_info(log_cpu, "TID: %i - FETCH - Program Counter: %i", tid, pc);
    t_instruccion* instruccion = malloc(sizeof(t_instruccion));
    op_code codigo = recibir_operacion(sockets_cpu->socket_memoria); // TODO ver como modelar la operacion
    if(codigo == READY){
        log_info(log_cpu, "COPOP: %i", codigo);
        instruccion = recibir_instruccion(sockets_cpu->socket_memoria); // 
    }else{
        return NULL;
    }
  return instruccion;
    
}

void pedir_instruccion_memoria(uint32_t tid, uint32_t pc, t_log *logg){
    t_paquete* paquete = crear_paquete_op(PEDIR_INSTRUCCION_MEMORIA);
    agregar_entero_a_paquete(paquete,tid);
    agregar_entero_a_paquete(paquete,pc);
    
    //log_info(logg, "serializacion %i %i", tid, pc); ya esta el log
    enviar_paquete(paquete,sockets_cpu->socket_memoria);
    eliminar_paquete(paquete);

}

op_code decode(t_instruccion *instruccion){
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

void execute(op_code instruccion_nombre, t_instruccion* instruccion){
    switch (instruccion_nombre) {
        case SET:
            log_info(log_cpu, "INSTRUCCION :%s - PARAMETRO 1: %s - PARAMETRO 2: %s", instruccion->parametros1, instruccion->parametros2, instruccion->parametros3);
            funcSET(instruccion->parametros2, instruccion->parametros3);
            break;
        case SUM:
            log_info(log_cpu, "INSTRUCCION :%s - PARAMETRO 1: %s - PARAMETRO 2: %s", instruccion->parametros1, instruccion->parametros2, instruccion->parametros3);
            funcSUM(instruccion->parametros2, instruccion->parametros3);
            break;
        case SUB:
            log_info(log_cpu, "INSTRUCCION :%s - PARAMETRO 1: %s - PARAMETRO 2: %s", instruccion->parametros1, instruccion->parametros2, instruccion->parametros3);
            funcSUB(instruccion->parametros2, instruccion->parametros3);
            break;
        case JNZ:
            log_info(log_cpu, "INSTRUCCION :%s - PARAMETRO 1: %s - PARAMETRO 2: %s", instruccion->parametros1, instruccion->parametros2, instruccion->parametros3);
            funcJNZ(instruccion->parametros2, instruccion->parametros3);
            break;
        case READ_MEM:
            log_info(log_cpu, "INSTRUCCION :%s - PARAMETRO 1: %s - PARAMETRO 2: %s", instruccion->parametros1, instruccion->parametros2, instruccion->parametros3);
            funcREAD_MEM(instruccion->parametros2, instruccion->parametros3);
            break;
        case WRITE_MEM:
            log_info(log_cpu, "INSTRUCCION :%s - PARAMETRO 1: %s - PARAMETRO 2: %s", instruccion->parametros1, instruccion->parametros2, instruccion->parametros3);
            funcWRITE_MEM(instruccion->parametros2, instruccion->parametros3);
            break;
        case LOG:
            log_info(log_cpu, "INSTRUCCION :%s - PARAMETRO 1: %s ", instruccion->parametros1, instruccion->parametros2);
            funcLOG(instruccion->parametros2);
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
                //enviar_contexto(sockets_cpu->socket_servidor->socket_Interrupt, contexto,INTERRUPCION); // EL CONTEXTO SE LE ENVÍA A MEMORIA NO A KERNEL
                enviar_contexto(sockets_cpu->socket_memoria,contexto,INTERRUPCION);
            }else{
                //enviar_contexto(sockets_cpu->socket_servidor->socket_Dispatch, contexto,INTERRUPCION_USUARIO); // EL CONTEXTO SE LE ENVÍA A MEMORIA NO A KERNEL
                enviar_contexto(sockets_cpu->socket_memoria,contexto,INTERRUPCION_USUARIO);
            }
        }
    }
}

// Recepción de mensajes de Kernel Dispatch
void recibir_kernel_dispatch(int socket_cliente_Dispatch) { 
    int noFinalizar = 0;
    while (noFinalizar != -1) {
        t_paquete_code_operacion*paquete=recibir_paquete_code_operacion(socket_cliente_Dispatch);
        //int codOperacion = recibir_operacion(socket_cliente_Dispatch);
        /*switch (codOperacion) {
            case EXEC:
                log_trace(log_cpu, "Ejecutando ciclo de instrucción.");
                contexto = recibir_contexto(socket_cliente_Dispatch);
                ciclo_de_instruccion(log_cpu);
                break;
            case -1:
                noFinalizar = codOperacion;
                break;
            default:
                break;
        }*/

        switch (paquete->code){
            case THREAD_EXECUTE_AVISO:
                /*Al momento de recibir un TID y PID de parte del Kernel la CPU deberá solicitarle el contexto de ejecución correspondiente a la Memoria para poder iniciar su ejecución.*/
                t_tid_pid*info = recepcionar_tid_pid_code_op(paquete);
                log_trace(log_cpu, "Ejecutando ciclo de instrucción.");
                //contexto = recibir_contexto(socket_cliente_Dispatch); // EL CONTEXTO SE RECIBE DE MEMORIA. NO DEL SOCKET DISPATCH
                contexto = recibir_contexto_para_thread_execute(sockets_cpu->socket_memoria,info->tid);
                ciclo_de_instruccion(log_cpu);

                break;
            case -1:
                noFinalizar = paquete->code;
                break;
            default:
                break;
        }

    }
}