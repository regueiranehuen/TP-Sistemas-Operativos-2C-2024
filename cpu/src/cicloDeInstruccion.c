#include "cicloDeInstruccion.h"
#include "funcExecute.h"
#include "mmu.h"
#include "server.h"

t_instruccion instruccion;
bool seguir_ejecutando;

void iniciar_cpu(){

    pthread_t hilo_ciclo_instruccion;
    pthread_t hilo_atiende_interrupt;
    

    int resultado;


    resultado=pthread_create(&hilo_ciclo_instruccion,NULL,ciclo_de_instruccion,NULL);

    if (resultado != 0)
    {
        log_error(log_cpu, "Error al crear el hilo que atiende los ciclos de instruccion en cpu");

    }

    resultado=pthread_create(&hilo_atiende_interrupt,NULL,recibir_kernel_interrupt,NULL);

    if (resultado != 0)
    {
        log_error(log_cpu, "Error al crear el hilo que atiende interrupt en cpu");

    }

    
    pthread_detach(hilo_ciclo_instruccion);
    pthread_detach(hilo_atiende_interrupt);
    
    
}

void *ciclo_de_instruccion(void *args)
{

    int noFinalizar = 0;

    while (noFinalizar != -1)
    {
        t_contextos *contextos = esperar_thread_execute(sockets_cpu->socket_servidor->socket_cliente_Dispatch);

        if (contextos->contexto_pid != NULL && contextos->contexto_tid != NULL)
        {
            seguir_ejecutando = true;
            while (seguir_ejecutando)
            {
                printf("ete sech");
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
            }
        }
    }
    return NULL;
}

t_contextos *esperar_thread_execute(int socket_cliente_Dispatch)
{
    log_info(log_cpu,"esperando thread execute\n");
    t_paquete_code_operacion *paquete = recibir_paquete_code_operacion(socket_cliente_Dispatch);
    
    if(paquete==NULL){
        printf("Cpu recibio un paquete no valido de kernel por dispatch");
        t_contextos* contextos = esperar_thread_execute(socket_cliente_Dispatch);
        return contextos;
    }
    
    log_info(log_cpu,"se recibió el código %d por dispatch\n",paquete->code);
    
    t_contextos *contextos = malloc(sizeof(t_contextos));
    contextos->contexto_pid = malloc(sizeof(t_contexto_pid_send));
    contextos->contexto_tid = malloc(sizeof(t_contexto_tid));

    if (paquete->code == THREAD_EXECUTE_AVISO)
    {
        /*Al momento de recibir un TID y PID de parte del Kernel la CPU deberá solicitarle el contexto de ejecución correspondiente a la Memoria para poder iniciar su ejecución.*/
        t_tid_pid *info = recepcionar_tid_pid_code_op(paquete);
        
        printf("pid:%d\n",info->pid);
        printf("tid:%d\n",info->tid);
    
        solicitar_contexto_pid(info->pid, sockets_cpu->socket_memoria);
        printf("ya solicite el contexto\n");
        t_paquete *paquete_solicitud_contexto_pid = recibir_paquete_op_code(sockets_cpu->socket_memoria);
        printf("Recibi solicitud de memoria\n");
        if (paquete_solicitud_contexto_pid->codigo_operacion == CONTEXTO_PID_INEXISTENTE)
        {
            log_error(log_cpu, "El contexto del pid %d no existe", info->pid);
        }
        else if (paquete_solicitud_contexto_pid->codigo_operacion == OBTENCION_CONTEXTO_PID_OK)
        {
            
            contextos->contexto_pid = recepcionar_contexto_pid(paquete_solicitud_contexto_pid);
            solicitar_contexto_tid(info->pid, info->tid, sockets_cpu->socket_memoria);
            log_info(log_cpu, "TID: %d - Solicito Contexto Ejecución", info->tid);
            log_info(log_cpu,"PID: %d, BASE: %d, LIMITE: %d",contextos->contexto_pid->pid,contextos->contexto_pid->base,contextos->contexto_pid->limite);
            t_paquete *paquete_solicitud_contexto_tid = recibir_paquete_op_code(sockets_cpu->socket_memoria);
            printf("%d\n",paquete_solicitud_contexto_tid->codigo_operacion);
            if (paquete_solicitud_contexto_tid->codigo_operacion == OBTENCION_CONTEXTO_TID_OK)
            { // La memoria se encarga de crear el contexto del tid si es que no existe
                contextos->contexto_tid = recepcionar_contexto_tid(paquete_solicitud_contexto_tid);
                log_info(log_cpu, "TID: %d - Solicito Contexto Ejecución", info->tid);
                printf("fentanilo:%d%d%d%d%d%d%d%d%d%d%d",contextos->contexto_tid->registros->AX,contextos->contexto_tid->registros->BX,contextos->contexto_tid->registros->CX,contextos->contexto_tid->registros->DX,contextos->contexto_tid->registros->EX,contextos->contexto_tid->registros->FX,contextos->contexto_tid->registros->GX,contextos->contexto_tid->registros->HX,contextos->contexto_tid->registros->PC,contextos->contexto_tid->pid,contextos->contexto_tid->tid);
            }
            else if (paquete_solicitud_contexto_tid->codigo_operacion == CONTEXTO_TID_INEXISTENTE)
            {
                log_error(log_cpu, "Error obteniendo contexto del tid %d", info->tid);
            }
        }
        else if (paquete_solicitud_contexto_pid->codigo_operacion == -1)
        {
            log_error(log_cpu, "Error obteniendo contexto del tid %d", info->pid);
        }

        log_trace(log_cpu, "Ejecutando ciclo de instrucción.");
    }

    return contextos;
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
        pthread_mutex_unlock(&mutex_interrupt);
        seguir_ejecutando = false;
        enviar_registros_a_actualizar(sockets_cpu->socket_memoria,contextoTid->registros,contextoTid->pid,contextoTid->tid);
        code_operacion respuesta = recibir_code_operacion(sockets_cpu->socket_memoria);
        if(respuesta != OK){
            log_info(log_cpu,"Memoria no pudo actualizar los registros, muy poco sigma");
            return;
        }
        if (devolucion_kernel == FIN_QUANTUM_RR){
            send_fin_quantum_rr(sockets_cpu->socket_servidor->socket_cliente_Dispatch);
        }
        else if (devolucion_kernel == DESALOJAR){
            send_desalojo(sockets_cpu->socket_servidor->socket_cliente_Dispatch);
        }
    }
    else{
        pthread_mutex_unlock(&mutex_interrupt);
    }
}


t_instruccion *fetch(t_contexto_tid *contexto){

    log_info(log_cpu,"ENTRAMOS A FETCH. Tid %d, program counter %d",contexto->tid,contexto->registros->PC);

    send_solicitud_instruccion_memoria(contexto->tid, contexto->pid, contexto->registros->PC);

    t_paquete *paquete = recibir_paquete_op_code(sockets_cpu->socket_memoria);
    t_instruccion *instruccion;
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

void send_solicitud_instruccion_memoria(int tid, int pid, uint32_t pc){
    t_buffer* buffer = malloc(sizeof(t_buffer));
    buffer->size = 2*sizeof(int) + sizeof(uint32_t);
    buffer->stream = malloc(buffer->size);

    void* stream = buffer->stream;

    memcpy(stream,&(pid),sizeof(int));
    stream += sizeof(int);
    memcpy(stream,&(tid),sizeof(int));
    stream += sizeof(int);
    memcpy(stream,&(pc),sizeof(uint32_t));    

    op_code code = OBTENER_INSTRUCCION;

    send_paquete_op_code(sockets_cpu->socket_memoria,buffer,code);
}

op_code decode(t_instruccion *instruccion){
    log_info(log_cpu,"ESTAMOS EN DECODE CON LA INSTRUCCIÓN %s",instruccion->parametros1);
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


void execute(t_contexto_pid_send *contextoPid,t_contexto_tid *contextoTid, op_code instruccion_nombre, t_instruccion *instruccion){
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
        
        send_dump_memory(sockets_cpu->socket_servidor->socket_cliente_Dispatch);
        
        contextoTid->registros->PC++;
        
        sem_wait(&sem_syscall_finalizada);
        break;
    case IO:
        log_info(log_cpu, "IO - Tiempo: %d", atoi(instruccion->parametros2));
        enviar_registros_a_actualizar(sockets_cpu->socket_memoria, contextoTid->registros, contextoTid->pid, contextoTid->tid);
        
        send_IO(atoi(instruccion->parametros2), sockets_cpu->socket_servidor->socket_cliente_Dispatch);
        
        contextoTid->registros->PC++;
        sem_wait(&sem_syscall_finalizada);
        break;
    case PROCESS_CREATE:
        log_info(log_cpu, "PROCESS_CREATE - PID: %s, Tamaño: %d, Prioridad: %d", instruccion->parametros2, atoi(instruccion->parametros3), atoi(instruccion->parametros4));
        enviar_registros_a_actualizar(sockets_cpu->socket_memoria, contextoTid->registros, contextoTid->pid, contextoTid->tid);
        
        send_process_create(instruccion->parametros2, atoi(instruccion->parametros3), atoi(instruccion->parametros4), sockets_cpu->socket_servidor->socket_cliente_Dispatch);
        
        contextoTid->registros->PC++;
        sem_wait(&sem_syscall_finalizada);
        break;
    case THREAD_CREATE:
        log_info(log_cpu, "THREAD_CREATE - TID: %s, Prioridad: %d", instruccion->parametros2, atoi(instruccion->parametros3));
        enviar_registros_a_actualizar(sockets_cpu->socket_memoria, contextoTid->registros, contextoTid->pid, contextoTid->tid);
        
        send_thread_create(instruccion->parametros2, atoi(instruccion->parametros3), sockets_cpu->socket_servidor->socket_cliente_Dispatch);
        
        contextoTid->registros->PC++;
        sem_wait(&sem_syscall_finalizada);
        break;
    case THREAD_JOIN:
        log_info(log_cpu, "THREAD_JOIN - TID: %d", atoi(instruccion->parametros2));
        enviar_registros_a_actualizar(sockets_cpu->socket_memoria, contextoTid->registros, contextoTid->pid, contextoTid->tid);
        
        send_thread_join(atoi(instruccion->parametros2), sockets_cpu->socket_servidor->socket_cliente_Dispatch);
        
        contextoTid->registros->PC++;
        sem_wait(&sem_syscall_finalizada);
        break;
    case THREAD_CANCEL:
        log_info(log_cpu, "THREAD_CANCEL - TID: %d", atoi(instruccion->parametros2));
        enviar_registros_a_actualizar(sockets_cpu->socket_memoria, contextoTid->registros, contextoTid->pid, contextoTid->tid);
        
        send_thread_cancel(atoi(instruccion->parametros2), sockets_cpu->socket_servidor->socket_cliente_Dispatch);
        
        contextoTid->registros->PC++;
        sem_wait(&sem_syscall_finalizada);
        break;
    case MUTEX_CREATE:
        log_info(log_cpu, "MUTEX_CREATE - Nombre: %s", instruccion->parametros2);
        enviar_registros_a_actualizar(sockets_cpu->socket_memoria, contextoTid->registros, contextoTid->pid, contextoTid->tid);
        
        send_mutex_create(instruccion->parametros2, sockets_cpu->socket_servidor->socket_cliente_Dispatch);
        
        contextoTid->registros->PC++;
        sem_wait(&sem_syscall_finalizada);
        break;
    case MUTEX_LOCK:
        log_info(log_cpu, "MUTEX_LOCK - Nombre: %s", instruccion->parametros2);
        enviar_registros_a_actualizar(sockets_cpu->socket_memoria, contextoTid->registros, contextoTid->pid, contextoTid->tid);
        
        send_mutex_lock(instruccion->parametros2, sockets_cpu->socket_servidor->socket_cliente_Dispatch);
        
        contextoTid->registros->PC++;
        sem_wait(&sem_syscall_finalizada);
        break;
    case MUTEX_UNLOCK:
        log_info(log_cpu, "MUTEX_UNLOCK - Nombre: %s", instruccion->parametros2);
        enviar_registros_a_actualizar(sockets_cpu->socket_memoria, contextoTid->registros, contextoTid->pid, contextoTid->tid);
        
        send_mutex_unlock(instruccion->parametros2, sockets_cpu->socket_servidor->socket_cliente_Dispatch);
        
        contextoTid->registros->PC++;
        sem_wait(&sem_syscall_finalizada);
        break;
    case THREAD_EXIT:
        log_info(log_cpu, "THREAD_EXIT");
        enviar_registros_a_actualizar(sockets_cpu->socket_memoria, contextoTid->registros, contextoTid->pid, contextoTid->tid);
        
        send_thread_exit(sockets_cpu->socket_servidor->socket_cliente_Dispatch);
        
        contextoTid->registros->PC++;
        sem_wait(&sem_syscall_finalizada);
        break;
    case PROCESS_EXIT:
        log_info(log_cpu, "PROCESS_EXIT");
        enviar_registros_a_actualizar(sockets_cpu->socket_memoria, contextoTid->registros, contextoTid->pid, contextoTid->tid);
        
        send_process_exit(sockets_cpu->socket_servidor->socket_servidor_Dispatch);
    
        contextoTid->registros->PC++;
        sem_wait(&sem_syscall_finalizada);
        break;
    default:
        log_error(log_cpu, "Instrucción no válida");
        break;
    }
}