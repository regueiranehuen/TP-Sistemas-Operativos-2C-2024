#include "cicloDeInstruccion.h"
#include "funcExecute.h"
#include "mmu.h"
#include "server.h"

t_instruccion instruccion;
bool seguir_ejecutando;



void ciclo_de_instruccion(t_contexto_pid* contextoPid, t_contexto_tid*contextoTid){
    seguir_ejecutando=true;
    //t_contexto_tid*contexto=obtener_contexto_tid(pid,tid);
    while (seguir_ejecutando){
        t_instruccion*instruccion=fetch(contextoTid);
        if (instruccion == NULL) {
            seguir_ejecutando = false;
            continue; // Si hay un error, salir del ciclo
        }
        op_code nombre_instruccion = decode(instruccion);
        execute(contextoPid,contextoTid, nombre_instruccion, instruccion);
        if (seguir_ejecutando){
            
            //checkInterrupt(contextoPid,contextoTid); // TODO REVISAR 
        }
    }
}

/*
En este momento, se deberá chequear si el Kernel nos envió una interrupción al TID que se está ejecutando, 
en caso afirmativo, se actualiza el Contexto de Ejecución en la Memoria y se devuelve el TID al Kernel con motivo de la interrupción. 
Caso contrario, se descarta la interrupción.
*/

/*void checkInterrupt(t_contexto_pid* contextoPid,t_contexto_tid* contextoTid->tid){ // Falta devolver al kernel el tid con motivo de interrupcion

    //wait
    if (hay_interrupcion){
        hay_interrupcion = false;
    //signal

        // wait
        if(tid == tid_interrupt){
        // signal

            //wait
            seguir_ejecutando = false;
            //signal

            //wait
            t_contexto_tid*contexto=obtener_contexto_tid(pid,tid);
            //signal
            if(contexto == NULL){
                printf("Error: no se pudo obtener el contexto del TID %d\n",tid);

                // signal para liberar el semaforo
                return;
            }

            actualizar_contexto_en_memoria(contexto);

            notificar_kernel_terminacion(tid,ENUM_SEGMENTATION_FAULT); //TODO VER DE DONDE SACAR EL TIPO DE INTERRUPCION

            // if(es_por_usuario){
                
            //     //enviar_contexto_tid(sockets_cpu->socket_memoria,contexto,INTERRUPCION);

            //     enviar_contexto_a_memoria(contexto);
                
            //     notificar_kernel_terminacion(tid,ENUM_SEGMENTATION_FAULT); // 
                

            // }
        }
    }
}

void actualizar_contexto_en_memoria(t_contexto_pid*contexto_pid){
    if (contexto == NULL)
    {
        printf("Error: El contexto proporcionado es nulo.\n");
        return;
    }

    t_contexto_tid* contexto_actual_tid = obtener_contexto_tid(contexto->pid,contexto->contextos_tids);

    if(contexto_actual != NULL){
        contexto_actual->registros->PC = contexto->contextos_tids->registros->PC;

    }else{
        // muerte No SE
    }
    
}
*/


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

void execute(t_contexto_pid*contextoPid,t_contexto_tid* contextoTid ,op_code instruccion_nombre, t_instruccion* instruccion) {
    log_info(log_cpu, "Ejecutando instrucción: %s", instruccion->parametros1);
    
    switch (instruccion_nombre) {
        case SET:
            log_info(log_cpu, "SET - Registro: %s, Valor: %d", instruccion->parametros2, atoi(instruccion->parametros3));
            funcSET(contextoTid,instruccion->parametros2, (uint32_t)atoi(instruccion->parametros3));
            modificar_registros(contextoTid);
            break;
        case SUM:
            log_info(log_cpu, "SUM - Registro: %s, Valor: %s", instruccion->parametros2, instruccion->parametros3);
            funcSUM(contextoTid,instruccion->parametros2, instruccion->parametros3);
            modificar_registros(contextoTid);
            break;
        case SUB:
            log_info(log_cpu, "SUB - Registro: %s, Valor: %s", instruccion->parametros2, instruccion->parametros3);
            funcSUB(contextoTid,instruccion->parametros2, instruccion->parametros3);
            modificar_registros(contextoTid);
            break;
        case JNZ:
            log_info(log_cpu, "JNZ - Registro: %s, Valor: %d", instruccion->parametros2, atoi(instruccion->parametros3));
            funcJNZ(contextoTid,instruccion->parametros2, (uint32_t)atoi(instruccion->parametros3));
            modificar_registros(contextoTid);
            break;
        case READ_MEM:
            log_info(log_cpu, "READ_MEM - Dirección: %s", instruccion->parametros2);
            funcREAD_MEM(contextoPid,contextoTid,instruccion->parametros2, instruccion->parametros3);
            modificar_registros(contextoTid);
            break;
        case WRITE_MEM:
            log_info(log_cpu, "WRITE_MEM - Dirección: %s, Valor: %s", instruccion->parametros2, instruccion->parametros3);
            funcWRITE_MEM(contextoPid,contextoTid,instruccion->parametros2, instruccion->parametros3);
            modificar_registros(contextoTid);
            break;
        case LOG:
            log_info(log_cpu, "LOG - Mensaje: %s", instruccion->parametros2);
            funcLOG(contextoTid,instruccion->parametros2);
            modificar_registros(contextoTid);
            break;
        case DUMP_MEMORY:
            log_info(log_cpu, "DUMP_MEMORY");
            send_dump_memory(sockets_cpu->socket_servidor->socket_Interrupt);
            esperar_ok_kernel(contextoTid);
            break;
        case IO:
            log_info(log_cpu, "IO - Tiempo: %d", atoi(instruccion->parametros2));
            send_IO(atoi(instruccion->parametros2), sockets_cpu->socket_servidor->socket_Interrupt);
            esperar_ok_kernel(contextoTid);
            break;
        case PROCESS_CREATE:
            log_info(log_cpu, "PROCESS_CREATE - PID: %s, Tamaño: %d, Prioridad: %d", instruccion->parametros2, atoi(instruccion->parametros3), atoi(instruccion->parametros4));
            send_process_create(instruccion->parametros2, atoi(instruccion->parametros3), atoi(instruccion->parametros4), sockets_cpu->socket_servidor->socket_Interrupt);
            esperar_ok_kernel(contextoTid);
            break;
        case THREAD_CREATE:
            log_info(log_cpu, "THREAD_CREATE - TID: %s, Prioridad: %d", instruccion->parametros2, atoi(instruccion->parametros3));
            send_thread_create(instruccion->parametros2, atoi(instruccion->parametros3), sockets_cpu->socket_servidor->socket_Interrupt);
            esperar_ok_kernel(contextoTid);
            break;
        case THREAD_JOIN:
            log_info(log_cpu, "THREAD_JOIN - TID: %d", atoi(instruccion->parametros2));
            send_thread_join(atoi(instruccion->parametros2), sockets_cpu->socket_servidor->socket_Interrupt);
            esperar_ok_kernel(contextoTid);
            break;
        case THREAD_CANCEL:
            log_info(log_cpu, "THREAD_CANCEL - TID: %d", atoi(instruccion->parametros2));
            send_thread_cancel(atoi(instruccion->parametros2), sockets_cpu->socket_servidor->socket_Interrupt);
            esperar_ok_kernel(contextoTid);
            break;
        case MUTEX_CREATE:
            log_info(log_cpu, "MUTEX_CREATE - Nombre: %s", instruccion->parametros2);
            send_mutex_create(instruccion->parametros2, sockets_cpu->socket_servidor->socket_Interrupt);
            esperar_ok_kernel(contextoTid);
            break;
        case MUTEX_LOCK:
            log_info(log_cpu, "MUTEX_LOCK - Nombre: %s", instruccion->parametros2);
            send_mutex_lock(instruccion->parametros2, sockets_cpu->socket_servidor->socket_Interrupt);
            esperar_ok_kernel(contextoTid);
            break;
        case MUTEX_UNLOCK:
            log_info(log_cpu, "MUTEX_UNLOCK - Nombre: %s", instruccion->parametros2);
            send_mutex_unlock(instruccion->parametros2, sockets_cpu->socket_servidor->socket_Interrupt);
            esperar_ok_kernel(contextoTid);
            break;
        case THREAD_EXIT:
            log_info(log_cpu, "THREAD_EXIT");
            send_thread_exit(sockets_cpu->socket_servidor->socket_Interrupt);
            esperar_ok_kernel(contextoTid);
            break;
        case PROCESS_EXIT:
            log_info(log_cpu, "PROCESS_EXIT");
            send_process_exit(sockets_cpu->socket_servidor->socket_Interrupt);
            esperar_ok_kernel(contextoTid);
            break;
        default:
            log_error(log_cpu, "Instrucción no válida");
            break;
    }
}

void modificar_registros(t_contexto_tid* contexto){
    contexto->registros->PC++;
    enviar_contexto_tid(sockets_cpu->socket_memoria,contexto,77); // NO SE QUE CODIGO MANDAR
}


void esperar_ok_kernel(t_contexto_tid*contexto){
    code_operacion code = recibir_code_operacion(sockets_cpu->socket_servidor->socket_Dispatch);
    if (code == OK){
        contexto->registros->PC++;
        enviar_contexto_tid(sockets_cpu->socket_memoria,contexto,77); // NO SE QUE CODIGO MANDAR
    }
        
}


