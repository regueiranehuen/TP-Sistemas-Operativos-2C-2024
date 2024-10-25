#include "cicloDeInstruccion.h"
#include "funcExecute.h"
#include "mmu.h"
#include "server.h"

t_instruccion instruccion;
bool seguir_ejecutando;

void ciclo_de_instruccion(t_contexto_pid *contextoPid, t_contexto_tid *contextoTid){
    seguir_ejecutando = true;
    while (seguir_ejecutando){
        t_instruccion *instruccion = fetch(contextoTid);
        if (instruccion == NULL){
            seguir_ejecutando = false;
            continue;
        }
        op_code nombre_instruccion = decode(instruccion);
        execute(contextoPid, contextoTid, nombre_instruccion, instruccion);
        if(seguir_ejecutando){
            checkInterrupt(contextoTid);
        }
    }
}

/*
En este momento, se deberá chequear si el Kernel nos envió una interrupción al TID que se está ejecutando,
en caso afirmativo, se actualiza el Contexto de Ejecución en la Memoria y se devuelve el TID al Kernel con motivo de la interrupción.
Caso contrario, se descarta la interrupción.
*/

void checkInterrupt(t_contexto_tid* contextoTid) {

    
    pthread_mutex_lock(&mutex_interrupt);
    
    if (hay_interrupcion) {
        hay_interrupcion = false;
        seguir_ejecutando = false;
        enviar_registros_a_actualizar(sockets_cpu->socket_memoria,contextoTid->registros,contextoTid->pid,contextoTid->tid);

        send_operacion_tid(devolucion_kernel,contextoTid->tid,sockets_cpu->socket_servidor->socket_Dispatch);
        
    }
    pthread_mutex_unlock(&mutex_interrupt);
}


t_instruccion *fetch(t_contexto_tid *contexto){
    pedir_instruccion_memoria(contexto->tid, contexto->pid, contexto->registros->PC);

    t_paquete *paquete = recibir_paquete_op_code(sockets_cpu->socket_memoria);
    t_instruccion *instruccion = NULL;
    if (paquete->codigo_operacion == -1)
    {
        log_error(log_cpu, "Error al recibir la instrucción");
        seguir_ejecutando = false;
        return NULL;
    }
    else if (paquete->codigo_operacion == INSTRUCCION_OBTENIDA)
    {
        instruccion = recepcionar_instruccion(paquete);
    }
    log_info(log_cpu, "TID: %i - FETCH - Program Counter: %i", contexto->tid, contexto->registros->PC);
    return instruccion;
}

void pedir_instruccion_memoria(int tid, int pid, uint32_t pc){
    t_paquete *paquete = crear_paquete_op(OBTENER_INSTRUCCION);
    agregar_entero_int_a_paquete(paquete, tid);
    agregar_entero_int_a_paquete(paquete, pid);
    agregar_entero_uint32_a_paquete(paquete, pc);

    enviar_paquete(paquete, sockets_cpu->socket_memoria);
    eliminar_paquete(paquete);
}

op_code decode(t_instruccion *instruccion){
    if (strcmp(instruccion->parametros1, "SET") == 0)
    {
        return SET;
    }
    else if (strcmp(instruccion->parametros1, "READ_MEM") == 0)
    {
        return READ_MEM;
    }
    else if (strcmp(instruccion->parametros1, "WRITE_MEM") == 0)
    {
        return WRITE_MEM;
    }
    else if (strcmp(instruccion->parametros1, "SUM") == 0)
    {
        return SUM;
    }
    else if (strcmp(instruccion->parametros1, "SUB") == 0)
    {
        return SUB;
    }
    else if (strcmp(instruccion->parametros1, "JNZ") == 0)
    {
        return JNZ;
    }
    else if (strcmp(instruccion->parametros1, "LOG") == 0)
    {
        return LOG;
    }
    else if (strcmp(instruccion->parametros1, "DUMP_MEMORY") == 0)
    {
        return DUMP_MEMORY;
    }
    else if (strcmp(instruccion->parametros1, "IO") == 0)
    {
        return IO;
    }
    else if (strcmp(instruccion->parametros1, "PROCESS_CREATE") == 0)
    {
        return PROCESS_CREATE;
    }
    else if (strcmp(instruccion->parametros1, "THREAD_CREATE") == 0)
    {
        return THREAD_CREATE;
    }
    else if (strcmp(instruccion->parametros1, "THREAD_JOIN") == 0)
    {
        return THREAD_JOIN;
    }
    else if (strcmp(instruccion->parametros1, "THREAD_CANCEL") == 0)
    {
        return THREAD_CANCEL;
    }
    else if (strcmp(instruccion->parametros1, "MUTEX_CREATE") == 0)
    {
        return MUTEX_CREATE;
    }
    else if (strcmp(instruccion->parametros1, "MUTEX_LOCK") == 0)
    {
        return MUTEX_LOCK;
    }
    else if (strcmp(instruccion->parametros1, "MUTEX_UNLOCK") == 0)
    {
        return MUTEX_UNLOCK;
    }
    else if (strcmp(instruccion->parametros1, "THREAD_EXIT") == 0)
    {
        return THREAD_EXIT;
    }
    else if (strcmp(instruccion->parametros1, "IO_FS_READ") == 0)
    {
        return PROCESS_EXIT;
    }

    return -1; // Código de operación no válido
}

// Durante el transcurso de la ejecución de un hilo, se irá actualizando su Contexto de Ejecución, que luego será devuelto a la Memoria bajo los siguientes escenarios: 
// finalización del mismo (PROCESS_EXIT o THREAD_EXIT), ejecutar una llamada al Kernel (syscall), deber ser desalojado (interrupción) o por la ocurrencia de un error Segmentation Fault.


void execute(t_contexto_pid *contextoPid,t_contexto_tid *contextoTid, op_code instruccion_nombre, t_instruccion *instruccion){
    log_info(log_cpu, "Ejecutando instrucción: %s", instruccion->parametros1);

    switch (instruccion_nombre){
    case SET:
        log_info(log_cpu, "SET - Registro: %s, Valor: %d", instruccion->parametros2, atoi(instruccion->parametros3));
        funcSET(contextoTid, instruccion->parametros2, (uint32_t)atoi(instruccion->parametros3));
        contextoTid->registros->PC++;
        break;
    case SUM:
        log_info(log_cpu, "SUM - Registro: %s, Valor: %s", instruccion->parametros2, instruccion->parametros3);
        funcSUM(contextoTid, instruccion->parametros2, instruccion->parametros3);
        contextoTid->registros->PC++;
        break;
    case SUB:
        log_info(log_cpu, "SUB - Registro: %s, Valor: %s", instruccion->parametros2, instruccion->parametros3);
        funcSUB(contextoTid, instruccion->parametros2, instruccion->parametros3);
        contextoTid->registros->PC++;
        break; 
    case JNZ:
        log_info(log_cpu, "JNZ - Registro: %s, Valor: %d", instruccion->parametros2, atoi(instruccion->parametros3));
        funcJNZ(contextoTid, instruccion->parametros2, (uint32_t)atoi(instruccion->parametros3));
        // No se incrementa el program counter
        break;
    case READ_MEM:
        log_info(log_cpu, "READ_MEM - Dirección: %s", instruccion->parametros2);
        funcREAD_MEM(contextoPid, contextoTid, instruccion->parametros2, instruccion->parametros3);
        contextoTid->registros->PC++;

        break;
    case WRITE_MEM:
        log_info(log_cpu, "WRITE_MEM - Dirección: %s, Valor: %s", instruccion->parametros2, instruccion->parametros3);
        funcWRITE_MEM(contextoPid, contextoTid, instruccion->parametros2, instruccion->parametros3);
        contextoTid->registros->PC++;

        break;
    case LOG:
        log_info(log_cpu, "LOG - Mensaje: %s", instruccion->parametros2);
        funcLOG(contextoTid, instruccion->parametros2);
        contextoTid->registros->PC++;

        break;
    case DUMP_MEMORY:
        log_info(log_cpu, "DUMP_MEMORY");
        enviar_registros_a_actualizar(sockets_cpu->socket_memoria, contextoTid->registros, contextoTid->pid, contextoTid->tid);
        
        send_dump_memory(sockets_cpu->socket_servidor->socket_Dispatch);
        
        contextoTid->registros->PC++;
        
        sem_wait(&sem_syscall_interrumpida_o_finalizada);
        break;
    case IO:
        log_info(log_cpu, "IO - Tiempo: %d", atoi(instruccion->parametros2));
        enviar_registros_a_actualizar(sockets_cpu->socket_memoria, contextoTid->registros, contextoTid->pid, contextoTid->tid);
        
        send_IO(atoi(instruccion->parametros2), sockets_cpu->socket_servidor->socket_Dispatch);
        
        contextoTid->registros->PC++;
        sem_wait(&sem_syscall_interrumpida_o_finalizada);
        break;
    case PROCESS_CREATE:
        log_info(log_cpu, "PROCESS_CREATE - PID: %s, Tamaño: %d, Prioridad: %d", instruccion->parametros2, atoi(instruccion->parametros3), atoi(instruccion->parametros4));
        enviar_registros_a_actualizar(sockets_cpu->socket_memoria, contextoTid->registros, contextoTid->pid, contextoTid->tid);
        
        send_process_create(instruccion->parametros2, atoi(instruccion->parametros3), atoi(instruccion->parametros4), sockets_cpu->socket_servidor->socket_Dispatch);
        
        contextoTid->registros->PC++;
        sem_wait(&sem_syscall_interrumpida_o_finalizada);
        break;
    case THREAD_CREATE:
        log_info(log_cpu, "THREAD_CREATE - TID: %s, Prioridad: %d", instruccion->parametros2, atoi(instruccion->parametros3));
        enviar_registros_a_actualizar(sockets_cpu->socket_memoria, contextoTid->registros, contextoTid->pid, contextoTid->tid);
        
        send_thread_create(instruccion->parametros2, atoi(instruccion->parametros3), sockets_cpu->socket_servidor->socket_Dispatch);
        
        contextoTid->registros->PC++;
        sem_wait(&sem_syscall_interrumpida_o_finalizada);
        break;
    case THREAD_JOIN:
        log_info(log_cpu, "THREAD_JOIN - TID: %d", atoi(instruccion->parametros2));
        enviar_registros_a_actualizar(sockets_cpu->socket_memoria, contextoTid->registros, contextoTid->pid, contextoTid->tid);
        
        send_thread_join(atoi(instruccion->parametros2), sockets_cpu->socket_servidor->socket_Dispatch);
        
        contextoTid->registros->PC++;
        sem_wait(&sem_syscall_interrumpida_o_finalizada);
        break;
    case THREAD_CANCEL:
        log_info(log_cpu, "THREAD_CANCEL - TID: %d", atoi(instruccion->parametros2));
        enviar_registros_a_actualizar(sockets_cpu->socket_memoria, contextoTid->registros, contextoTid->pid, contextoTid->tid);
        
        send_thread_cancel(atoi(instruccion->parametros2), sockets_cpu->socket_servidor->socket_Dispatch);
        
        contextoTid->registros->PC++;
        sem_wait(&sem_syscall_interrumpida_o_finalizada);
        break;
    case MUTEX_CREATE:
        log_info(log_cpu, "MUTEX_CREATE - Nombre: %s", instruccion->parametros2);
        enviar_registros_a_actualizar(sockets_cpu->socket_memoria, contextoTid->registros, contextoTid->pid, contextoTid->tid);
        
        send_mutex_create(instruccion->parametros2, sockets_cpu->socket_servidor->socket_Dispatch);
        
        contextoTid->registros->PC++;
        sem_wait(&sem_syscall_interrumpida_o_finalizada);
        break;
    case MUTEX_LOCK:
        log_info(log_cpu, "MUTEX_LOCK - Nombre: %s", instruccion->parametros2);
        enviar_registros_a_actualizar(sockets_cpu->socket_memoria, contextoTid->registros, contextoTid->pid, contextoTid->tid);
        
        send_mutex_lock(instruccion->parametros2, sockets_cpu->socket_servidor->socket_Dispatch);
        
        contextoTid->registros->PC++;
        sem_wait(&sem_syscall_interrumpida_o_finalizada);
        break;
    case MUTEX_UNLOCK:
        log_info(log_cpu, "MUTEX_UNLOCK - Nombre: %s", instruccion->parametros2);
        enviar_registros_a_actualizar(sockets_cpu->socket_memoria, contextoTid->registros, contextoTid->pid, contextoTid->tid);
        
        send_mutex_unlock(instruccion->parametros2, sockets_cpu->socket_servidor->socket_Dispatch);
        
        contextoTid->registros->PC++;
        sem_wait(&sem_syscall_interrumpida_o_finalizada);
        break;
    case THREAD_EXIT:
        log_info(log_cpu, "THREAD_EXIT");
        enviar_registros_a_actualizar(sockets_cpu->socket_memoria, contextoTid->registros, contextoTid->pid, contextoTid->tid);
        
        send_thread_exit(sockets_cpu->socket_servidor->socket_Dispatch);
        
        contextoTid->registros->PC++;
        sem_wait(&sem_syscall_interrumpida_o_finalizada);
        break;
    case PROCESS_EXIT:
        log_info(log_cpu, "PROCESS_EXIT");
        enviar_registros_a_actualizar(sockets_cpu->socket_memoria, contextoTid->registros, contextoTid->pid, contextoTid->tid);
        
        send_process_exit(sockets_cpu->socket_servidor->socket_Dispatch);
        
        contextoTid->registros->PC++;
        sem_wait(&sem_syscall_interrumpida_o_finalizada);
        break;
    default:
        log_error(log_cpu, "Instrucción no válida");
        break;
    }
}
