#include "cicloDeInstruccion.h"
#include "funcExecute.h"
#include "mmu.h"
#include "server.h"

t_instruccion instruccion;
bool seguir_ejecutando;


t_contexto_pid*contexto_pid_exec;
t_contexto_tid*contexto_tid_exec;

int tid_exec;
int pid_exec;


void ciclo_de_instruccion(int tid, int pid){
    seguir_ejecutando=true;
    t_contexto_tid*contexto=obtener_contexto_tid(pid,tid);
    while (seguir_ejecutando){
        t_instruccion*instruccion=fetch(contexto);
        if (instruccion == NULL) {
            seguir_ejecutando = false;
            continue; // Si hay un error, salir del ciclo
        }
        op_code nombre_instruccion = decode(instruccion);
        execute(nombre_instruccion,instruccion);

        if (seguir_ejecutando){
            checkInterrupt(tid,pid);
        }
    }
}




void checkInterrupt(int tid,int pid){ // Falta devolver al kernel el tid con motivo de interrupcion
    if (hay_interrupcion){
        hay_interrupcion = false;
        if(tid == tid_interrupt){
            seguir_ejecutando = false;
            if(!es_por_usuario){
                obtener_contexto_tid(pid,tid);
                //enviar_contexto_tid(sockets_cpu->socket_memoria,contexto,INTERRUPCION);

                

            }else{
                obtener_contexto_tid(pid_exec,tid_exec);
                //enviar_contexto_tid(sockets_cpu->socket_memoria,contexto,INTERRUPCION_USUARIO);
            }
        }
    }
}



t_instruccion* fetch(t_contexto_tid*contexto){
    pedir_instruccion_memoria(contexto->tid,contexto->registros->PC,log_cpu);
    log_info(log_cpu, "TID: %i - FETCH - Program Counter: %i", contexto->tid, contexto->registros->PC);

    t_instruccion*instruccion=recibir_instruccion(sockets_cpu->socket_memoria);

    if (instruccion == NULL) {
        log_error(log_cpu, "Error al recibir la instrucción");
        seguir_ejecutando = false;
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

void execute(op_code instruccion_nombre, t_instruccion* instruccion) {
    log_info(log_cpu, "Ejecutando instrucción: %s", instruccion->parametros1);
    
    switch (instruccion_nombre) {
        case SET:
            log_info(log_cpu, "SET - Registro: %s, Valor: %d", instruccion->parametros2, atoi(instruccion->parametros3));
            funcSET(instruccion->parametros2, (uint32_t)atoi(instruccion->parametros3));
            contexto->pc++;
            break;
        case SUM:
            log_info(log_cpu, "SUM - Registro: %s, Valor: %s", instruccion->parametros2, instruccion->parametros3);
            funcSUM(instruccion->parametros2, instruccion->parametros3);
            contexto->pc++;
            break;
        case SUB:
            log_info(log_cpu, "SUB - Registro: %s, Valor: %s", instruccion->parametros2, instruccion->parametros3);
            funcSUB(instruccion->parametros2, instruccion->parametros3);
            contexto->pc++;
            break;
        case JNZ:
            log_info(log_cpu, "JNZ - Registro: %s, Valor: %d", instruccion->parametros2, atoi(instruccion->parametros3));
            funcJNZ(instruccion->parametros2, (uint32_t)atoi(instruccion->parametros3));
            contexto->pc++;
            break;
        case READ_MEM:
            log_info(log_cpu, "READ_MEM - Dirección: %s", instruccion->parametros2);
            funcREAD_MEM(instruccion->parametros2, instruccion->parametros3);
            contexto->pc++;
            break;
        case WRITE_MEM:
            log_info(log_cpu, "WRITE_MEM - Dirección: %s, Valor: %s", instruccion->parametros2, instruccion->parametros3);
            funcWRITE_MEM(instruccion->parametros2, instruccion->parametros3);
            contexto->pc++;
            break;
        case LOG:
            log_info(log_cpu, "LOG - Mensaje: %s", instruccion->parametros2);
            funcLOG(instruccion->parametros2);
            contexto->pc++;
            break;
        case DUMP_MEMORY:
            log_info(log_cpu, "DUMP_MEMORY");
            send_dump_memory(sockets_cpu->socket_servidor->socket_Interrupt);
            enviar_contexto_a_memoria(sockets_cpu->socket_memoria, contexto);
            break;
        case IO:
            log_info(log_cpu, "IO - Tiempo: %d", atoi(instruccion->parametros2));
            send_IO(atoi(instruccion->parametros2), sockets_cpu->socket_servidor->socket_Interrupt);
            enviar_contexto_a_memoria(sockets_cpu->socket_memoria, contexto);
            break;
        case PROCESS_CREATE:
            log_info(log_cpu, "PROCESS_CREATE - PID: %s, Tamaño: %d, Prioridad: %d", instruccion->parametros2, atoi(instruccion->parametros3), atoi(instruccion->parametros4));
            send_process_create(instruccion->parametros2, atoi(instruccion->parametros3), atoi(instruccion->parametros4), sockets_cpu->socket_servidor->socket_Interrupt);
            enviar_contexto_a_memoria(sockets_cpu->socket_memoria, contexto);
            break;
        case THREAD_CREATE:
            log_info(log_cpu, "THREAD_CREATE - TID: %s, Prioridad: %d", instruccion->parametros2, atoi(instruccion->parametros3));
            send_thread_create(instruccion->parametros2, atoi(instruccion->parametros3), sockets_cpu->socket_servidor->socket_Interrupt);
            enviar_contexto_a_memoria(sockets_cpu->socket_memoria, contexto);
            break;
        case THREAD_JOIN:
            log_info(log_cpu, "THREAD_JOIN - TID: %d", atoi(instruccion->parametros2));
            send_thread_join(atoi(instruccion->parametros2), sockets_cpu->socket_servidor->socket_Interrupt);
            enviar_contexto_a_memoria(sockets_cpu->socket_memoria, contexto);
            break;
        case THREAD_CANCEL:
            log_info(log_cpu, "THREAD_CANCEL - TID: %d", atoi(instruccion->parametros2));
            send_thread_cancel(atoi(instruccion->parametros2), sockets_cpu->socket_servidor->socket_Interrupt);
            enviar_contexto_a_memoria(sockets_cpu->socket_memoria, contexto);
            break;
        case MUTEX_CREATE:
            log_info(log_cpu, "MUTEX_CREATE - Nombre: %s", instruccion->parametros2);
            send_mutex_create(instruccion->parametros2, sockets_cpu->socket_servidor->socket_Interrupt);
            enviar_contexto_a_memoria(sockets_cpu->socket_memoria, contexto);
            break;
        case MUTEX_LOCK:
            log_info(log_cpu, "MUTEX_LOCK - Nombre: %s", instruccion->parametros2);
            send_mutex_lock(instruccion->parametros2, sockets_cpu->socket_servidor->socket_Interrupt);
            enviar_contexto_a_memoria(sockets_cpu->socket_memoria, contexto);
            break;
        case MUTEX_UNLOCK:
            log_info(log_cpu, "MUTEX_UNLOCK - Nombre: %s", instruccion->parametros2);
            send_mutex_unlock(instruccion->parametros2, sockets_cpu->socket_servidor->socket_Interrupt);
            enviar_contexto_a_memoria(sockets_cpu->socket_memoria, contexto);
            break;
        case THREAD_EXIT:
            log_info(log_cpu, "THREAD_EXIT");
            send_thread_exit(sockets_cpu->socket_servidor->socket_Interrupt);
            enviar_contexto_a_memoria(sockets_cpu->socket_memoria, contexto);
            break;
        case PROCESS_EXIT:
            log_info(log_cpu, "PROCESS_EXIT");
            send_process_exit(sockets_cpu->socket_servidor->socket_Interrupt);
            enviar_contexto_a_memoria(sockets_cpu->socket_memoria, contexto);
            seguir_ejecutando = 0;
            break;
        default:
            log_error(log_cpu, "Instrucción no válida");
            break;
    }
}

void analizar_terminacion(void){
    code_operacion code = recibir_code_operacion(sockets_cpu->socket_servidor->socket_Interrupt);
    if (code == TERMINAR)
        seguir_ejecutando=false;
}



// Recepción de mensajes de Kernel Dispatch
void recibir_kernel_dispatch(int socket_cliente_Dispatch) { // Juntar recv de dipatch y de interrupt
    int noFinalizar = 0;
    while (noFinalizar != -1) {
        t_paquete_code_operacion*paquete=recibir_paquete_code_operacion(socket_cliente_Dispatch);
        switch (paquete->code){
            case THREAD_EXECUTE_AVISO:
                /*Al momento de recibir un TID y PID de parte del Kernel la CPU deberá solicitarle el contexto de ejecución correspondiente a la Memoria para poder iniciar su ejecución.*/
                t_tid_pid*info = recepcionar_tid_pid_code_op(paquete);

                t_contexto_tid*contexto=obtener_contexto_tid(info->pid,info->tid);

                if (contexto == NULL){
                    t_contexto_pid* contexto_pid = obtener_contexto_pid(info->pid,info->tid);
                    contexto= inicializar_contexto_tid(contexto_pid,info->tid);
                    list_add(contexto_pid->contextos_tids,contexto);
                }
            
                log_trace(log_cpu, "Ejecutando ciclo de instrucción.");
                //MUTEX_LOCK
                tid_exec=info->tid;
                pid_exec=info->pid;
                //MUTEX_UNLOCK
               
                ciclo_de_instruccion(info->tid,info->pid); // OJO CON ESTA FUNCION. ACÁ DEBERÍA MANDARSE LA VARIABLE CONTEXTO DE ARRIBA Y NO LA GLOBAL
            case OK:
                noFinalizar=0; 
                break;
            default:
                break;
        }
    }
}