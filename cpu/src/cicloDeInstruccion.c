#include "cicloDeInstruccion.h"
#include "funcExecute.h"
#include "mmu.h"
#include "server.h"

t_instruccion instruccion;
bool seguir_ejecutando;



void iniciar_cpu(){

    int resultado;
    pthread_t hilo_ciclo_instruccion;
    pthread_t hilo_atiende_interrupt;
    resultado = pthread_create(&hilo_ciclo_instruccion, NULL, ciclo_de_instruccion, NULL);

    if (resultado != 0)
    {
        pthread_mutex_lock(&mutex_logs);
        log_error(log_cpu, "Error al crear el hilo que atiende los ciclos de instruccion en cpu");
        pthread_mutex_unlock(&mutex_logs);
    }

    resultado = pthread_create(&hilo_atiende_interrupt, NULL, recibir_kernel_interrupt, NULL);

    if (resultado != 0)
    {
        pthread_mutex_lock(&mutex_logs);
        log_error(log_cpu, "Error al crear el hilo que atiende interrupt en cpu");
        pthread_mutex_unlock(&mutex_logs);
    }

    pthread_detach(hilo_ciclo_instruccion);
    pthread_detach(hilo_atiende_interrupt);
    
}

void *ciclo_de_instruccion(void *args)
{

    int noFinalizar = 0;

    while (noFinalizar != -1)
    {
        send_ciclo_nuevo(sockets_cpu->socket_servidor->socket_cliente_Dispatch);
        t_contextos *contextos = esperar_thread_execute(sockets_cpu->socket_servidor->socket_cliente_Dispatch);

        if (contextos != NULL && contextos->contexto_pid != NULL && contextos->contexto_tid != NULL)
        {
            seguir_ejecutando = true;
            while (seguir_ejecutando == true)
            {
                
                t_instruccion *instruccion = fetch(contextos->contexto_tid);
                if (instruccion == NULL)
                {
                    
                    seguir_ejecutando = false;
                    continue;
                }
                
                op_code nombre_instruccion = decode(instruccion);
                
                execute(contextos->contexto_pid, contextos->contexto_tid, nombre_instruccion, instruccion);
                if (seguir_ejecutando)
                {
                    checkInterrupt(contextos->contexto_tid);
                }
                
                free(instruccion->parametros1);
                free(instruccion->parametros2);
                free(instruccion->parametros3);
                free(instruccion->parametros4);
                free(instruccion);
            }
            free(contextos->contexto_tid->registros);
            free(contextos->contexto_tid);
            free(contextos->contexto_pid);
            free(contextos);
        }
        else if(contextos == NULL){
            pthread_mutex_lock(&mutex_logs);
            log_info(log_cpu,"Cierre de conexion con kernel");
            pthread_mutex_unlock(&mutex_logs);
            sem_post(&sem_finalizacion_cpu);
            return NULL; 
        }
    }
    return NULL;
}

t_contextos *esperar_thread_execute(int socket_cliente_Dispatch)
{
    
    t_paquete_code_operacion *paquete = recibir_paquete_code_operacion(socket_cliente_Dispatch);

    if (paquete == NULL)
    {
        pthread_mutex_lock(&mutex_logs);
        log_info(log_cpu,"Cpu recibio un paquete no valido de kernel por dispatch");
        pthread_mutex_unlock(&mutex_logs);
        return NULL;
    }

    if (paquete->code == THREAD_EXECUTE_AVISO)
    {
        /*Al momento de recibir un TID y PID de parte del Kernel la CPU deberá solicitarle el contexto de ejecución correspondiente a la Memoria para poder iniciar su ejecución.*/
        t_tid_pid *info = recepcionar_tid_pid_code_op(paquete);

        t_contextos* contextos;

        solicitar_contexto_ejecucion(info->pid, info->tid,sockets_cpu->socket_memoria);

        pthread_mutex_lock(&mutex_logs);
        log_debug(log_cpu,"## TID: %d - Solicito Contexto Ejecución",info->tid);
        pthread_mutex_unlock(&mutex_logs);
        
        t_paquete *paquete_solicitud_contexto_ejecucion = recibir_paquete_op_code(sockets_cpu->socket_memoria);
        
        if (paquete_solicitud_contexto_ejecucion->codigo_operacion == ERROR)
        {
            pthread_mutex_lock(&mutex_logs);
            log_error(log_cpu, "El contexto del pid %d tid %d no existe", info->pid,info->tid);
            pthread_mutex_unlock(&mutex_logs);
            return NULL;
        }
        else if (paquete_solicitud_contexto_ejecucion->codigo_operacion == OBTENCION_CONTEXTO_EJECUCION_OK)
        {
            contextos = recepcionar_contextos(paquete_solicitud_contexto_ejecucion);
        }
        pthread_mutex_lock(&mutex_logs);
        log_trace(log_cpu, "Ejecutando ciclo de instrucción.");
        pthread_mutex_unlock(&mutex_logs);

        free(info);

        return contextos;
    }
    else{
        return NULL;
    }

    return NULL;
}

/*
En este momento, se deberá chequear si el Kernel nos envió una interrupción al TID que se está ejecutando,
en caso afirmativo, se actualiza el Contexto de Ejecución en la Memoria y se devuelve el TID al Kernel con motivo de la interrupción.
Caso contrario, se descarta la interrupción.
*/

void checkInterrupt(t_contexto_tid *contextoTid)
{

    pthread_mutex_lock(&mutex_interrupt);

    if (hay_interrupcion)
    {
        hay_interrupcion = false;
        seguir_ejecutando = false;
    
        enviar_registros_a_actualizar(sockets_cpu->socket_memoria, contextoTid->registros, contextoTid->pid, contextoTid->tid);
        pthread_mutex_lock(&mutex_logs);
        log_debug(log_cpu,"## TID: %d - Actualizo Contexto Ejecución",contextoTid->tid);
        pthread_mutex_unlock(&mutex_logs);
        code_operacion respuesta = recibir_code_operacion(sockets_cpu->socket_memoria);
        if (respuesta != OK)
        {
            pthread_mutex_unlock(&mutex_interrupt);
            return;
        }
        if (devolucion_kernel == FIN_QUANTUM_RR)
        {
            pthread_mutex_lock(&mutex_logs);
            log_info(log_cpu,"MANDANDO FIN QUANTUM A KERNEL\n");
            pthread_mutex_unlock(&mutex_logs);
            send_syscall(ENUM_FIN_QUANTUM_RR,sockets_cpu->socket_servidor->socket_cliente_Interrupt);
        }
        else if (devolucion_kernel == DESALOJAR)
        {
            pthread_mutex_lock(&mutex_logs);
            log_info(log_cpu,"Mandando desalojo a kernel\n");
            pthread_mutex_unlock(&mutex_logs);
            send_syscall(ENUM_DESALOJAR,sockets_cpu->socket_servidor->socket_cliente_Interrupt);
        }
        pthread_mutex_unlock(&mutex_interrupt);
    }
    else
    {
        pthread_mutex_lock(&mutex_logs);
        log_info(log_cpu,"NO HAY INTERRUPCIONES PARA EL TID %d PID %d PROGRAM COUNTER %d, seguimos",contextoTid->tid,contextoTid->pid,contextoTid->registros->PC);
        pthread_mutex_unlock(&mutex_logs);
        pthread_mutex_unlock(&mutex_interrupt);
    }
}

t_instruccion *fetch(t_contexto_tid *contexto)
{
    pthread_mutex_lock(&mutex_logs);
    log_info(log_cpu,"ENTRAMOS A FETCH CON PID %d TID %d PC %d",contexto->pid,contexto->tid,contexto->registros->PC);
    pthread_mutex_unlock(&mutex_logs);

    send_solicitud_instruccion_memoria(contexto->tid, contexto->pid, contexto->registros->PC);

    t_paquete *paquete = recibir_paquete_op_code(sockets_cpu->socket_memoria);
    t_instruccion *instruccion;
    if (paquete->codigo_operacion == -1)
    {
        pthread_mutex_lock(&mutex_logs);
        log_error(log_cpu, "Error al recibir la instrucción");
        pthread_mutex_unlock(&mutex_logs);
        seguir_ejecutando = false;
        eliminar_paquete(paquete);
        return NULL;
    }
    else if (paquete->codigo_operacion == INSTRUCCION_OBTENIDA)
    {
        instruccion = recepcionar_instruccion(paquete); 
    }
    pthread_mutex_lock(&mutex_logs);
    log_debug(log_cpu, "## TID: %d - FETCH - Program Counter: %u", contexto->tid, contexto->registros->PC);
    pthread_mutex_unlock(&mutex_logs);
    return instruccion;
}

void send_solicitud_instruccion_memoria(int tid, int pid, uint32_t pc)
{
    t_buffer *buffer = malloc(sizeof(t_buffer));
    buffer->size = 2 * sizeof(int) + sizeof(uint32_t);
    buffer->stream = malloc(buffer->size);

    void *stream = buffer->stream;

    memcpy(stream, &(pid), sizeof(int));
    stream += sizeof(int);
    memcpy(stream, &(tid), sizeof(int));
    stream += sizeof(int);
    memcpy(stream, &(pc), sizeof(uint32_t));

    op_code code = OBTENER_INSTRUCCION;

    send_paquete_op_code(sockets_cpu->socket_memoria, buffer, code);
}

op_code decode(t_instruccion *instruccion)
{
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
    else if (strcmp(instruccion->parametros1, "PROCESS_EXIT") == 0)
    {
        return PROCESS_EXIT;
    }

    return -1; // Código de operación no válido
}

// Durante el transcurso de la ejecución de un hilo, se irá actualizando su Contexto de Ejecución, que luego será devuelto a la Memoria bajo los siguientes escenarios:
// finalización del mismo (PROCESS_EXIT o THREAD_EXIT), ejecutar una llamada al Kernel (syscall), deber ser desalojado (interrupción) o por la ocurrencia de un error Segmentation Fault.

void execute(t_contexto_pid_send *contextoPid, t_contexto_tid *contextoTid, op_code instruccion_nombre, t_instruccion *instruccion)
{
    pthread_mutex_lock(&mutex_logs);
    log_debug(log_cpu, "## TID: %d - Ejecutando: %s - %s %s %s", contextoTid->tid, instruccion->parametros1,instruccion->parametros2,instruccion->parametros3,instruccion->parametros4);
    pthread_mutex_unlock(&mutex_logs);
    
    switch (instruccion_nombre)
    {
    case SET:
        pthread_mutex_lock(&mutex_logs);
        log_info(log_cpu, "SET - Registro: %s, Valor: %d", instruccion->parametros2, atoi(instruccion->parametros3));
        pthread_mutex_unlock(&mutex_logs);
        funcSET(contextoTid, instruccion->parametros2, (uint32_t)atoi(instruccion->parametros3));
        if (strcmp(instruccion->parametros2, "PC") != 0)
            contextoTid->registros->PC++;
        break;
    case SUM:
        pthread_mutex_lock(&mutex_logs);
        log_info(log_cpu, "SUM - Registro: %s, Valor: %s", instruccion->parametros2, instruccion->parametros3);
        pthread_mutex_unlock(&mutex_logs);
        funcSUM(contextoTid, instruccion->parametros2, instruccion->parametros3);
        contextoTid->registros->PC++;
        break;
    case SUB:
        pthread_mutex_lock(&mutex_logs);
        log_info(log_cpu, "SUB - Registro: %s, Valor: %s", instruccion->parametros2, instruccion->parametros3);
        pthread_mutex_unlock(&mutex_logs);
        funcSUB(contextoTid, instruccion->parametros2, instruccion->parametros3);
        contextoTid->registros->PC++;
        break;
    case JNZ:
        pthread_mutex_lock(&mutex_logs);
        log_info(log_cpu, "JNZ - Registro: %s, Valor: %d", instruccion->parametros2, atoi(instruccion->parametros3));
        pthread_mutex_unlock(&mutex_logs);
        funcJNZ(contextoTid, instruccion->parametros2, (uint32_t)atoi(instruccion->parametros3));
        // El valor del program counter se evalúa dentro de la instrucción
        break;
    case READ_MEM:
        pthread_mutex_lock(&mutex_logs);
        log_info(log_cpu, "READ_MEM - Dirección: %s", instruccion->parametros2);
        pthread_mutex_unlock(&mutex_logs);
        funcREAD_MEM(contextoPid, contextoTid, instruccion->parametros2, instruccion->parametros3);
        contextoTid->registros->PC++;

        break;
    case WRITE_MEM:
        pthread_mutex_lock(&mutex_logs);
        log_info(log_cpu, "WRITE_MEM - Dirección: %s, Valor: %s", instruccion->parametros2, instruccion->parametros3);
        pthread_mutex_unlock(&mutex_logs);
        funcWRITE_MEM(contextoPid, contextoTid, instruccion->parametros2, instruccion->parametros3);
        contextoTid->registros->PC++;

        break;
    case LOG:
        pthread_mutex_lock(&mutex_logs);
        log_info(log_cpu, "LOG - Mensaje: %s", instruccion->parametros2);
        pthread_mutex_unlock(&mutex_logs);
        funcLOG(contextoTid, instruccion->parametros2);
        contextoTid->registros->PC++;

        break;
    case DUMP_MEMORY:
    {
        pthread_mutex_lock(&mutex_logs);
        log_info(log_cpu, "DUMP_MEMORY");
        pthread_mutex_unlock(&mutex_logs);
        enviar_registros_a_actualizar(sockets_cpu->socket_memoria, contextoTid->registros, contextoTid->pid, contextoTid->tid);
        code_operacion code = recibir_code_operacion(sockets_cpu->socket_memoria);
        if (code != OK)
        {
            pthread_mutex_lock(&mutex_logs);
            log_info(log_cpu, "Memoria no actualizo el registro correctamente");
            pthread_mutex_unlock(&mutex_logs);
        }

        send_dump_memory(sockets_cpu->socket_servidor->socket_cliente_Dispatch);

        pthread_mutex_lock(&mutex_logs);
        log_info(log_cpu, "DUMP_MEMORY ENVIADO");
        pthread_mutex_unlock(&mutex_logs);

        recibir_code_operacion(sockets_cpu->socket_servidor->socket_cliente_Dispatch);
        
        contextoTid->registros->PC++;

        break;
    }
    case IO:
    {
        pthread_mutex_lock(&mutex_logs);
        log_info(log_cpu, "IO - Tiempo: %d", atoi(instruccion->parametros2));
        pthread_mutex_unlock(&mutex_logs);
        enviar_registros_a_actualizar(sockets_cpu->socket_memoria, contextoTid->registros, contextoTid->pid, contextoTid->tid);
        code_operacion code = recibir_code_operacion(sockets_cpu->socket_memoria);
        if (code != OK)
        {
            pthread_mutex_lock(&mutex_logs);
            log_info(log_cpu, "Memoria no actualizo el registro correctamente");
            pthread_mutex_unlock(&mutex_logs);
        }
    
        send_IO(atoi(instruccion->parametros2), sockets_cpu->socket_servidor->socket_cliente_Dispatch);
        pthread_mutex_lock(&mutex_logs);
        log_info(log_cpu, "IO ENVIADO");
        pthread_mutex_unlock(&mutex_logs);

        recibir_code_operacion(sockets_cpu->socket_servidor->socket_cliente_Dispatch);
    
        contextoTid->registros->PC++;

        break;
    }
    case PROCESS_CREATE:
    {
        pthread_mutex_lock(&mutex_logs);
        log_info(log_cpu, "PROCESS_CREATE - PSEUDOCODIGO:%s, Tamaño: %d, Prioridad: %d", instruccion->parametros2, atoi(instruccion->parametros3), atoi(instruccion->parametros4));
        pthread_mutex_unlock(&mutex_logs);
        enviar_registros_a_actualizar(sockets_cpu->socket_memoria, contextoTid->registros, contextoTid->pid, contextoTid->tid);
        code_operacion code = recibir_code_operacion(sockets_cpu->socket_memoria);
        if (code != OK)
        {
            pthread_mutex_lock(&mutex_logs);
            log_info(log_cpu, "Memoria no actualizo el registro correctamente");
            pthread_mutex_unlock(&mutex_logs);
        }

        send_process_create(instruccion->parametros2, atoi(instruccion->parametros3), atoi(instruccion->parametros4), sockets_cpu->socket_servidor->socket_cliente_Dispatch);
        pthread_mutex_lock(&mutex_logs);
        log_info(log_cpu, "PROCESS_CREATE ENVIADO");
        pthread_mutex_unlock(&mutex_logs);

        recibir_code_operacion(sockets_cpu->socket_servidor->socket_cliente_Dispatch);
    
        contextoTid->registros->PC++;

        break;
    }
    case THREAD_CREATE:
    {
        pthread_mutex_lock(&mutex_logs);
        log_info(log_cpu, "THREAD_CREATE - PSEUDOCODIGO: %s, Prioridad: %d", instruccion->parametros2, atoi(instruccion->parametros3));
        pthread_mutex_unlock(&mutex_logs);
        enviar_registros_a_actualizar(sockets_cpu->socket_memoria, contextoTid->registros, contextoTid->pid, contextoTid->tid);
        code_operacion code = recibir_code_operacion(sockets_cpu->socket_memoria);
        if (code != OK)
        {
            pthread_mutex_lock(&mutex_logs);
            log_info(log_cpu, "Memoria no actualizo el registro correctamente");
            pthread_mutex_unlock(&mutex_logs);
        }

        send_thread_create(instruccion->parametros2, atoi(instruccion->parametros3), sockets_cpu->socket_servidor->socket_cliente_Dispatch);
        pthread_mutex_lock(&mutex_logs);
        log_info(log_cpu, "THREAD_CREATE ENVIADO");
        pthread_mutex_unlock(&mutex_logs);

        recibir_code_operacion(sockets_cpu->socket_servidor->socket_cliente_Dispatch);

        contextoTid->registros->PC++;

        break;
    }
    case THREAD_JOIN:
    {
        pthread_mutex_lock(&mutex_logs);
        log_info(log_cpu, "THREAD_JOIN - TID: %d", atoi(instruccion->parametros2));
        pthread_mutex_unlock(&mutex_logs);
        enviar_registros_a_actualizar(sockets_cpu->socket_memoria, contextoTid->registros, contextoTid->pid, contextoTid->tid);
        code_operacion code = recibir_code_operacion(sockets_cpu->socket_memoria);
        if (code != OK)
        {
            pthread_mutex_lock(&mutex_logs);
            log_info(log_cpu, "Memoria no actualizo el registro correctamente");
            pthread_mutex_unlock(&mutex_logs);
        }

        send_thread_join(atoi(instruccion->parametros2), sockets_cpu->socket_servidor->socket_cliente_Dispatch);
        pthread_mutex_lock(&mutex_logs);
        log_info(log_cpu, "THREAD_JOIN ENVIADO");
        pthread_mutex_unlock(&mutex_logs);

        recibir_code_operacion(sockets_cpu->socket_servidor->socket_cliente_Dispatch);
        
        contextoTid->registros->PC++;

        break;
    }
    case THREAD_CANCEL:
    {
        pthread_mutex_lock(&mutex_logs);
        log_info(log_cpu, "THREAD_CANCEL - TID: %d", atoi(instruccion->parametros2));
        pthread_mutex_unlock(&mutex_logs);
        enviar_registros_a_actualizar(sockets_cpu->socket_memoria, contextoTid->registros, contextoTid->pid, contextoTid->tid);
        code_operacion code = recibir_code_operacion(sockets_cpu->socket_memoria);
        if (code != OK)
        {
            pthread_mutex_lock(&mutex_logs);
            log_info(log_cpu, "Memoria no actualizo el registro correctamente");
            pthread_mutex_unlock(&mutex_logs);
        }

        send_thread_cancel(atoi(instruccion->parametros2), sockets_cpu->socket_servidor->socket_cliente_Dispatch);

        pthread_mutex_lock(&mutex_logs);
        log_info(log_cpu, "THREAD_CANCEL ENVIADO");
        pthread_mutex_unlock(&mutex_logs);

        recibir_code_operacion(sockets_cpu->socket_servidor->socket_cliente_Dispatch);
        
        contextoTid->registros->PC++;

        break;
    }
    case MUTEX_CREATE:
    {
        pthread_mutex_lock(&mutex_logs);
        log_info(log_cpu, "MUTEX_CREATE - Nombre: %s", instruccion->parametros2);
        pthread_mutex_unlock(&mutex_logs);

        enviar_registros_a_actualizar(sockets_cpu->socket_memoria, contextoTid->registros, contextoTid->pid, contextoTid->tid);
        code_operacion code = recibir_code_operacion(sockets_cpu->socket_memoria);
        if (code != OK)
        {
            pthread_mutex_lock(&mutex_logs);
            log_info(log_cpu, "Memoria no actualizo el registro correctamente");
            pthread_mutex_unlock(&mutex_logs);
        }

        send_mutex_create(instruccion->parametros2, sockets_cpu->socket_servidor->socket_cliente_Dispatch);
        pthread_mutex_lock(&mutex_logs);
        log_info(log_cpu, "MUTEX_CREATE ENVIADO");
        pthread_mutex_unlock(&mutex_logs);

        recibir_code_operacion(sockets_cpu->socket_servidor->socket_cliente_Dispatch);
        
        contextoTid->registros->PC++;

        break;
    }
    case MUTEX_LOCK:
    {
        pthread_mutex_lock(&mutex_logs);
        log_info(log_cpu, "MUTEX_LOCK - Nombre: %s", instruccion->parametros2);
        pthread_mutex_unlock(&mutex_logs);
        enviar_registros_a_actualizar(sockets_cpu->socket_memoria, contextoTid->registros, contextoTid->pid, contextoTid->tid);
        code_operacion code = recibir_code_operacion(sockets_cpu->socket_memoria);
        if (code != OK)
        {
            pthread_mutex_lock(&mutex_logs);
            log_info(log_cpu, "Memoria no actualizo el registro correctamente");
            pthread_mutex_unlock(&mutex_logs);
        }

        send_mutex_lock(instruccion->parametros2, sockets_cpu->socket_servidor->socket_cliente_Dispatch);

        pthread_mutex_lock(&mutex_logs);
        log_info(log_cpu, "MUTEX_LOCK ENVIADO");
        pthread_mutex_unlock(&mutex_logs);

        recibir_code_operacion(sockets_cpu->socket_servidor->socket_cliente_Dispatch);
        
        contextoTid->registros->PC++;

        break;
    }
    case MUTEX_UNLOCK:
    {
        pthread_mutex_lock(&mutex_logs);
        log_info(log_cpu, "MUTEX_UNLOCK - Nombre: %s", instruccion->parametros2);
        pthread_mutex_unlock(&mutex_logs);
        enviar_registros_a_actualizar(sockets_cpu->socket_memoria, contextoTid->registros, contextoTid->pid, contextoTid->tid);

        code_operacion code = recibir_code_operacion(sockets_cpu->socket_memoria);
        if (code != OK)
        {
            pthread_mutex_lock(&mutex_logs);
            log_info(log_cpu, "Memoria no actualizo el registro correctamente");
            pthread_mutex_unlock(&mutex_logs);
        }

        send_mutex_unlock(instruccion->parametros2, sockets_cpu->socket_servidor->socket_cliente_Dispatch);

        pthread_mutex_lock(&mutex_logs);
        log_info(log_cpu, "MUTEX_UNLOCK ENVIADO");
        pthread_mutex_unlock(&mutex_logs);

        recibir_code_operacion(sockets_cpu->socket_servidor->socket_cliente_Dispatch);
        
        contextoTid->registros->PC++;

        break;
    }
    case THREAD_EXIT:
    {
        pthread_mutex_lock(&mutex_logs);
        log_info(log_cpu, "THREAD_EXIT");
        pthread_mutex_unlock(&mutex_logs);
        enviar_registros_a_actualizar(sockets_cpu->socket_memoria, contextoTid->registros, contextoTid->pid, contextoTid->tid);
        code_operacion code = recibir_code_operacion(sockets_cpu->socket_memoria);
        if (code != OK)
        {
            pthread_mutex_lock(&mutex_logs);
            log_info(log_cpu, "Memoria no actualizo el registro correctamente");
            pthread_mutex_unlock(&mutex_logs);
        }

        send_thread_exit(sockets_cpu->socket_servidor->socket_cliente_Dispatch);

        pthread_mutex_lock(&mutex_logs);
        log_info(log_cpu, "THREAD_EXIT ENVIADO");
        pthread_mutex_unlock(&mutex_logs);

        recibir_code_operacion(sockets_cpu->socket_servidor->socket_cliente_Dispatch);
        
        contextoTid->registros->PC++;
        break;
    }
    case PROCESS_EXIT:
    {
        pthread_mutex_lock(&mutex_logs);
        log_info(log_cpu, "PROCESS_EXIT");
        pthread_mutex_unlock(&mutex_logs);
        
        enviar_registros_a_actualizar(sockets_cpu->socket_memoria, contextoTid->registros, contextoTid->pid, contextoTid->tid);
        code_operacion code = recibir_code_operacion(sockets_cpu->socket_memoria);
        
        if (code != OK)
        {
            pthread_mutex_lock(&mutex_logs);
            log_info(log_cpu, "Memoria no actualizo el registro correctamente");
            pthread_mutex_unlock(&mutex_logs);
        }
        
        send_process_exit(sockets_cpu->socket_servidor->socket_cliente_Dispatch);
        

        recibir_code_operacion(sockets_cpu->socket_servidor->socket_cliente_Dispatch);

        break;
    }
    default:
        pthread_mutex_lock(&mutex_logs);
        log_error(log_cpu, "Instrucción no válida");
        pthread_mutex_unlock(&mutex_logs);
        break;
    }
}