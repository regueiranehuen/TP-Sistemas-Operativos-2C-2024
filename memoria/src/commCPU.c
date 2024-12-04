#include "includes/commCpu.h"

void *recibir_cpu(void *args)
{

    int retardo_respuesta = config_get_int_value(config, "RETARDO_RESPUESTA");
    int codigoOperacion = 0;
    while (codigoOperacion != -1)
    {

        t_paquete *paquete_operacion = recibir_paquete_op_code(sockets_iniciales->socket_cpu);

        if (paquete_operacion == NULL)
        {
            log_info(logger,"Memoria recibio un paquete == NULL de cpu. Conexion cerrada");

            break;
        }
        log_info(logger, "Memoria recibio el siguiente codigo operacion de CPU: %d", paquete_operacion->codigo_operacion);

        switch (paquete_operacion->codigo_operacion)
        {
        
        case OBTENER_CONTEXTO_EJECUCION:
        {
            t_tid_pid *info = recepcionar_solicitud_contexto(paquete_operacion); // Recibe PID y TID

            log_info(logger, "## Contexto solicitado - (PID:TID) - (%d:%d)", info->pid, info->tid);
            pthread_mutex_lock(&mutex_lista_contextos_pids);
            t_contextos* contextos = obtener_contextos(info->pid,info->tid);
            pthread_mutex_unlock(&mutex_lista_contextos_pids);

            

            if (contextos == NULL)
            {
                log_error(logger, "No se encontro el contexto del tid %d asociado al pid %d", info->tid, info->pid);
                op_code code = ERROR;
                send(sockets_iniciales->socket_cpu,&code,sizeof(int),0);
                free(info);
                return NULL;
            }
            free(info);
            enviar_contexto_ejecucion(contextos,sockets_iniciales->socket_cpu);
            log_info(logger, "Enviado contexto para PID: %d, TID: %d", contextos->contexto_tid->pid, contextos->contexto_tid->tid);

            /*for (int i = 0; i < list_size(lista_contextos_pids); i++)
            {
                t_contexto_pid *cont_pid_act = list_get(lista_contextos_pids, i);
                log_info(logger, "CONTEXTO PID:%d", cont_pid_act->pid);
                log_info(logger, "SIZE CONTEXTO TID:%d", list_size(cont_pid_act->contextos_tids));
                for (int j = 0; j < list_size(cont_pid_act->contextos_tids); j++)
                {
                    t_contexto_tid *cont_tid_act = list_get(cont_pid_act->contextos_tids, j);
                    log_info(logger, "TID:%d", cont_tid_act->tid);
                    log_info(logger, "AX: %d", cont_tid_act->registros->AX);
                }
            }*/
            free(contextos->contexto_pid);
            free(contextos); 

            break;
        }        

        case ACTUALIZAR_CONTEXTO_TID:
        {
            printf("entrando a actualizar_contexto\n");
            t_contexto_tid *contexto_tid = recepcionar_contexto_tid(paquete_operacion);

            log_info(logger, "REGISTROS QUE VOY A METER EN MEMORIA (ACTUALIZO):");
            log_info(logger, "AX: %u", contexto_tid->registros->AX);
            log_info(logger, "BX: %u", contexto_tid->registros->BX);
            log_info(logger, "CX: %u", contexto_tid->registros->CX);
            log_info(logger, "DX: %u", contexto_tid->registros->DX);
            log_info(logger, "EX: %u", contexto_tid->registros->EX);
            log_info(logger, "FX: %u", contexto_tid->registros->FX);
            log_info(logger, "GX: %u", contexto_tid->registros->GX);
            log_info(logger, "HX: %u", contexto_tid->registros->HX);            

            pthread_mutex_lock(&mutex_lista_contextos_pids);
            for (int i = 0; i < list_size(lista_contextos_pids); i++)
            {
                t_contexto_pid *cont_pid_act = list_get(lista_contextos_pids, i);
                log_info(logger, "CONTEXTO PID:%d", cont_pid_act->pid);
                log_info(logger, "SIZE CONTEXTO TID:%d", list_size(cont_pid_act->contextos_tids));
                for (int j = 0; j < list_size(cont_pid_act->contextos_tids); j++)
                {
                    t_contexto_tid *cont_tid_act = list_get(cont_pid_act->contextos_tids, j);
                    log_info(logger, "TID:%d", cont_tid_act->tid);
                    log_info(logger, "AX: %d", cont_tid_act->registros->AX);
                }
            }
                print_pids(lista_contextos_pids);
                pthread_mutex_unlock(&mutex_lista_contextos_pids);

                log_info(logger, "PROGRAM COUNTER ACTUAL: %u", contexto_tid->registros->PC);
                actualizar_contexto(contexto_tid->pid, contexto_tid->tid, contexto_tid->registros);
                send_code_operacion(OK, sockets_iniciales->socket_cpu);

                free(contexto_tid->registros); // Ya se actualizó el contexto que está en la lista de contextos tids correspondiente al pid enviado
                free(contexto_tid);
                break;
            }

        case OBTENER_INSTRUCCION:
        {
            log_info(logger, "Entramos a obtener instruccion, los parámetros que llegaron de solicitud son estos:");
            t_instruccion_memoria *solicitud_instruccion = recepcionar_solicitud_instruccion_memoria(paquete_operacion);
            log_info(logger, "%d", solicitud_instruccion->pid);
            log_info(logger, "%d", solicitud_instruccion->tid);
            log_info(logger, "%d", solicitud_instruccion->pc);
            t_instruccion *instruccion = obtener_instruccion(solicitud_instruccion->tid, solicitud_instruccion->pid, solicitud_instruccion->pc);
            if (instruccion == NULL)
            {
                log_info(logger, "instruccion no encontrada");
                free(solicitud_instruccion);
                break;
            }
            else
            {
                log_info(logger, "super instruccion a enviar: %s %s %s ", instruccion->parametros1, instruccion->parametros2, instruccion->parametros3);
            }
            enviar_instruccion(sockets_iniciales->socket_cpu, instruccion, INSTRUCCION_OBTENIDA);
            if (strcmp(instruccion->parametros2, "") == 0)
            {
                log_info(logger, "## Obtener instruccion - (PID:TID) - (%d:%d) - Instruccion: %s", solicitud_instruccion->pid, solicitud_instruccion->tid, instruccion->parametros1);
            }
            else if (strcmp(instruccion->parametros3, "") == 0)
            {
                log_info(logger, "## Obtener instrucción - (PID:TID) - (%d:%d) - Instrucción: %s %s", solicitud_instruccion->pid, solicitud_instruccion->tid, instruccion->parametros1, instruccion->parametros2);
            }
            else if (strcmp(instruccion->parametros4, "") == 0)
            {
                log_info(logger, "## Obtener instrucción - (PID:TID) - (%d:%d) - Instrucción: %s %s %s", solicitud_instruccion->pid, solicitud_instruccion->tid, instruccion->parametros1, instruccion->parametros2, instruccion->parametros3);
            }
            else
            {
                log_info(logger, "## Obtener instrucción - (PID:TID) - (%d:%d) - Instrucción: %s %s %s %s", solicitud_instruccion->pid, solicitud_instruccion->tid, instruccion->parametros1, instruccion->parametros2, instruccion->parametros3, instruccion->parametros4);
            }
            free(solicitud_instruccion);
            break;
        }

        case READ_MEM:
        {
            log_info(logger, "VOY A RECEPCIONAR EL READ MEM");
            uint32_t direccionFisica = recepcionar_read_mem(paquete_operacion);
            log_info(logger, "direccionFisica recibida:%d", direccionFisica);
            uint32_t valor = leer_Memoria(direccionFisica);
            log_info(logger, "valor leido de memoria:%d", valor);
            op_code code;
            if (valor == 0xFFFFFFFF)
            {
                code = ERROR;
                send_valor_read_mem(valor, sockets_iniciales->socket_cpu, code);
                log_info(logger, "entre a error");
            }
            else
            {
                code = OK_OP_CODE;
                send_valor_read_mem(valor, sockets_iniciales->socket_cpu, code);
                log_info(logger, "entre a ok");
            }
            break;
        }

        case WRITE_MEM:
        {
            t_write_mem *info_0 = recepcionar_write_mem(paquete_operacion);
            log_info(logger, "direccion Fisica recibida:%d", info_0->direccionFisica);
            log_info(logger, "valor recibido:%d", info_0->valor);
            int resultado = escribir_Memoria(info_0);
            if (resultado == 0)
            {
                op_code code = OK_OP_CODE;
                send(sockets_iniciales->socket_cpu, &code, sizeof(int), 0);
            }
            free(info_0);
            break;
        }

        case 1:
            codigoOperacion = paquete_operacion->codigo_operacion;
            free(paquete_operacion->buffer);
            free(paquete_operacion);
            break;

        default:
            free(paquete_operacion->buffer);
            free(paquete_operacion);
            log_warning(logger, "Operación desconocida recibida: %d", paquete_operacion->codigo_operacion);
            break;
        }

        usleep(retardo_respuesta * 1000); // Aplicar retardo configurado
    }
    log_warning(logger, "Se desconectó la CPU");

    int socket_filesystem = cliente_memoria_filesystem(logger, config);
    send_terminar_ejecucion(socket_filesystem);

    code_operacion code = recibir_code_operacion(socket_filesystem);
    close(socket_filesystem);

    if (code == OK_TERMINAR)
    {
        log_info(logger, "Se termina la ejecución del módulo memoria");
        free(memoria);
        liberar_lista_particiones(lista_particiones);
        sem_post(&sem_fin_memoria);
        return NULL;
    }
    else
    {
        log_info(logger, "SOY UN ESTORBO");
    }
    return NULL;
}

void actualizar_contexto(int pid, int tid, t_registros_cpu* reg){
    pthread_mutex_lock(&mutex_lista_contextos_pids);
    t_contexto_tid *contexto = obtener_contexto_tid(pid, tid);
    if(contexto == NULL){
        log_info(logger,"No se actualizó el contexto (PID:TID) - (%d:%d)",pid,tid);
    }
    if (contexto != NULL)
    {
        contexto->registros->AX = reg->AX;
        contexto->registros->BX = reg->BX;
        contexto->registros->CX = reg->CX;
        contexto->registros->DX = reg->DX;
        contexto->registros->EX = reg->EX;
        contexto->registros->FX = reg->FX;
        contexto->registros->GX = reg->GX;
        contexto->registros->HX = reg->HX;
        contexto->registros->PC = reg->PC;
        log_info(logger,"## Contexto Actualizado - (PID:TID) - (%d:%d)",pid,tid);
    }
    

    pthread_mutex_unlock(&mutex_lista_contextos_pids);
}

// Se utiliza al inicializar el contexto del tid 0 tras crear un nuevo proceso y cuando viene un hilo con tid N a ejecutar
t_contexto_tid* inicializar_contexto_tid(t_contexto_pid* cont,int tid){
    t_contexto_tid* contexto=malloc(sizeof(t_contexto_tid));
    contexto->registros= malloc(sizeof(t_registros_cpu));
    contexto->pid=cont->pid;
    contexto->tid = tid;
    contexto->registros->AX = 0;
    contexto->registros->BX = 0;
    contexto->registros->CX = 0;
    contexto->registros->DX = 0;
    contexto->registros->EX = 0;
    contexto->registros->FX = 0;
    contexto->registros->GX = 0;
    contexto->registros->HX = 0;
    contexto->registros->PC = 0;

    list_add(cont->contextos_tids,contexto);
    return contexto;
}

// Se debe usar despues de un PROCESS_CREATE y para el proceso inicial de la CPU
t_contexto_pid*inicializar_contexto_pid(int pid,uint32_t base, uint32_t limite,int tamanio_proceso){ 
    t_contexto_pid*nuevo_contexto=malloc(sizeof(t_contexto_pid));
    nuevo_contexto->pid=pid;
    nuevo_contexto->base=base;
    nuevo_contexto->limite=limite;
    nuevo_contexto->tamanio_proceso = tamanio_proceso;

    nuevo_contexto->contextos_tids=list_create();
// Paso como segundo parámetro el 0 ya que el proceso está siendo inicializado, y al iniciarse si o si tiene que tener un hilo
    t_contexto_tid* contexto_tid_0=inicializar_contexto_tid(nuevo_contexto,0); 
    if (contexto_tid_0!=NULL){

        pthread_mutex_lock(&mutex_lista_contextos_pids);
        list_add(lista_contextos_pids,nuevo_contexto);
        pthread_mutex_unlock(&mutex_lista_contextos_pids);

        return nuevo_contexto;
    }
    else{
        log_info(logger,"NO SE PUDO INICIALIZAR EL CONTEXTO DEL PID %d\n",pid);
    }
    return NULL;
    
}

t_contexto_tid*obtener_contexto_tid(int pid, int tid){ // hay que usar mutex cada vez que se usa esta funcion
    //pthread_mutex_lock(&mutex_lista_contextos_pids);
    t_contexto_pid* cont_pid = obtener_contexto_pid(pid);

    if (cont_pid!= NULL)
    {
        for (int i = 0; i < list_size(cont_pid->contextos_tids); i++)
        {
            t_contexto_tid *act = list_get(cont_pid->contextos_tids, i);
            log_info(logger, "TID EN OBT CONT TID: %d  CORRESPONDE A PID %d", act->tid, pid);
            if (act->tid == tid)
            {
                return act;
            }
        }
    }

    //pthread_mutex_unlock(&mutex_lista_contextos_pids);
    return NULL;
}

t_contextos*obtener_contextos(int pid, int tid){ // hay que usar mutex cada vez que se usa esta funcion

    t_contexto_pid* cont_pid = obtener_contexto_pid(pid);
    
    if (cont_pid!= NULL)
    {
        t_contextos* contextos = malloc(sizeof(t_contextos));
        contextos->contexto_pid = malloc(sizeof(t_contexto_pid_send));

        contextos->contexto_pid->base = cont_pid->base;
        contextos->contexto_pid->limite = cont_pid->limite;
        contextos->contexto_pid->pid = cont_pid->pid;
        contextos->contexto_pid->tamanio_proceso = cont_pid->tamanio_proceso;

        contextos->contexto_tid=malloc(sizeof(t_contexto_tid));
        contextos->contexto_tid->registros=malloc(sizeof(t_registros_cpu));
        for (int i = 0; i < list_size(cont_pid->contextos_tids); i++)
        {
            t_contexto_tid *act = list_get(cont_pid->contextos_tids, i);
            log_info(logger, "TID EN OBT CONT TID: %d  CORRESPONDE A PID %d", act->tid, pid);
            if (act->tid == tid)
            {
                
                contextos->contexto_tid->pid = act->pid;
                contextos->contexto_tid->tid=act->tid;
                contextos->contexto_tid->registros->AX=act->registros->AX;
                contextos->contexto_tid->registros->BX=act->registros->BX;
                contextos->contexto_tid->registros->CX=act->registros->CX;
                contextos->contexto_tid->registros->DX=act->registros->DX;
                contextos->contexto_tid->registros->EX=act->registros->EX;
                contextos->contexto_tid->registros->FX=act->registros->FX;
                contextos->contexto_tid->registros->GX=act->registros->GX;
                contextos->contexto_tid->registros->HX=act->registros->HX;
                contextos->contexto_tid->registros->PC=act->registros->PC;
                return contextos;
            }
        }
        free(contextos->contexto_tid->registros);
        free(contextos->contexto_tid);
        free(contextos->contexto_pid);
        free(contextos);

    }


    return NULL;
}


void remover_contexto_pid_lista(t_contexto_pid* contexto){
    for (int i = 0; i < list_size(lista_contextos_pids); i++){
        t_contexto_pid*cont_actual=(t_contexto_pid*)list_get(lista_contextos_pids,i);
        if (contexto->pid == cont_actual->pid){
            list_remove(lista_contextos_pids,i);
            break;
        }
            

    }
}

t_contexto_pid* obtener_contexto_pid(int pid){ // al usarla hay q meterle mutex x la lista de contextos
    //pthread_mutex_lock(&mutex_lista_contextos_pids);
    for (int i = 0; i < list_size(lista_contextos_pids); i++){
        t_contexto_pid*cont_actual=(t_contexto_pid*)list_get(lista_contextos_pids,i);
        log_warning(logger,"PID DEL CONT ACTUAL: %d",cont_actual->pid);
        if (pid == cont_actual->pid){
            //pthread_mutex_unlock(&mutex_lista_contextos_pids);
            return cont_actual;
        }
    }
    //pthread_mutex_unlock(&mutex_lista_contextos_pids);
    return NULL;
}


void remover_contexto_tid_lista(t_contexto_tid*contexto,t_list*lista){
    for (int i = 0; i < list_size(lista); i++){
        t_contexto_tid*cont_actual=(t_contexto_tid*)list_get(lista,i);
        if (contexto->tid == cont_actual->tid)
            list_remove(lista,i);

    }
}


bool esta_tid_en_lista(int tid,t_list*contextos_tids){
    for (int i = 0; i < list_size(contextos_tids); i++){
        t_contexto_tid*cont_actual=(t_contexto_tid*)list_get(contextos_tids,i);
        if (cont_actual->tid==tid)
            return true;

    }
    return false;
}

t_contexto_tid* obtener_tid_en_lista(int tid,t_list*contextos_tids){
    for (int i = 0; i < list_size(contextos_tids); i++){
        t_contexto_tid*cont_actual=(t_contexto_tid*)list_get(contextos_tids,i);
        if (cont_actual->tid==tid)
            return cont_actual;

    }
    return NULL;
}


void liberar_contexto_tid(t_contexto_pid *contexto_pid,t_contexto_tid*contexto_tid){
    if(contexto_tid){
        remover_contexto_tid_lista(contexto_tid,contexto_pid->contextos_tids);
        free(contexto_tid->registros);
        free(contexto_tid);
    }
}

void liberar_contexto_pid(t_contexto_pid *contexto_pid){
    if(contexto_pid){
        
        list_destroy_and_destroy_elements(contexto_pid->contextos_tids,(void*)liberar_contexto_tid);
        free(contexto_pid);
    }
}

void liberar_lista_contextos(){
    list_destroy_and_destroy_elements(lista_contextos_pids,(void*)liberar_contexto_pid);
}
